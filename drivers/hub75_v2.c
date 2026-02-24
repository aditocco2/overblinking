#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "images/test_gradient.h"
#include "hub75_v2.pio.h"

#define DATA_BASE_PIN 0
#define DATA_N_PINS 6
#define ROWSEL_BASE_PIN 6
#define ROWSEL_N_PINS 5
#define CLK_PIN 11
#define STROBE_PIN 12
#define OEN_PIN 13

#define WIDTH 64
#define HEIGHT 64

// pio state machines
#define DATA_SM 0
#define ROW_SM 1

static PIO pio = pio0;
// locations in memory of PIO programs
static uint32_t data_prog_offset;
static uint32_t row_prog_offset;

// The back buffer in our case right now is the test gradient from the header file
static uint16_t *back_buffer;
// this one is actually sent to the screen and has RGB888 bitplanes
static uint32_t front_buffer[WIDTH * HEIGHT];

static uint32_t bytes_per_bitplane = WIDTH * HEIGHT/2;

static uint32_t refresh_count;
static absolute_time_t ts;

static uint32_t pulse_width_buffer[] = {
   10, 20, 40, 80, 160, 320, 640, 1280
};

void hub75_configure();
void hub75_load_image();
void hub75_push();
void hub75_begin();
void make_bitplanes(uint16_t *in_565, uint8_t *out_888);
inline uint32_t rgb565_to_rgb888(uint16_t pix);
void dma_complete();

void main(void){
    stdio_init_all();
    hub75_configure();
    hub75_load_image(test_gradient);
    hub75_push();
    hub75_begin();
    while(1){
        // print refresh count every second
        if (absolute_time_diff_us(ts, get_absolute_time()) > 1000000){
            ts = get_absolute_time();
            printf("Refresh rate: %d Hz\n", refresh_count);
            refresh_count = 0;
        }
    }
}

int rgb_dma_channel, pw_dma_channel;
dma_channel_config rgb_dma_config, pw_dma_config;

void hub75_configure(){
    // Get the locations (offsets) of the PIO programs in memory
    data_prog_offset = pio_add_program(pio, &hub75_data_program);
    row_prog_offset = pio_add_program(pio, &hub75_row_program);

    // initialize PIO programs, passing in screen width to the data program
    hub75_data_program_init(pio, DATA_SM, data_prog_offset, DATA_BASE_PIN, CLK_PIN, WIDTH);
    hub75_row_program_init(pio, ROW_SM, row_prog_offset, ROWSEL_BASE_PIN, ROWSEL_N_PINS, STROBE_PIN);

    // initial setup of both DMA channels
    rgb_dma_channel = dma_claim_unused_channel(true);
    rgb_dma_config = dma_channel_get_default_config(rgb_dma_channel);
    channel_config_set_transfer_data_size(&rgb_dma_config, DMA_SIZE_32);
    channel_config_set_read_increment(&rgb_dma_config, true);
    channel_config_set_write_increment(&rgb_dma_config, false);
    channel_config_set_dreq(&rgb_dma_config, pio_get_dreq(pio, DATA_SM, true));

    pw_dma_channel = dma_claim_unused_channel(true);
    pw_dma_config = dma_channel_get_default_config(pw_dma_channel);
    channel_config_set_transfer_data_size(&pw_dma_config, DMA_SIZE_32);
    channel_config_set_read_increment(&pw_dma_config, true);
    channel_config_set_write_increment(&pw_dma_config, false);
    channel_config_set_dreq(&pw_dma_config, pio_get_dreq(pio, ROW_SM, true));
}

void hub75_load_image(uint16_t * image_pointer){
    back_buffer = image_pointer;
}

void hub75_push(){
    // upscale to RGB888 and make bitplanes in the front buffer
    make_bitplanes(back_buffer, (uint8_t *)front_buffer);
}

void hub75_begin(){
    // Make RGB DMA trigger an IRQ on completion
    dma_channel_set_irq0_enabled(rgb_dma_channel, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_complete);
    irq_set_enabled(DMA_IRQ_0, true); 

    // Make RGB DMA copy from front buffer to Data SM FIFO
    dma_channel_configure(
        rgb_dma_channel, &rgb_dma_config,
        &pio->txf[DATA_SM], front_buffer,
        WIDTH * HEIGHT, false
    );

    // Make PW DMA copy from pulse buffer to Row SM FIFO
    dma_channel_configure(
        pw_dma_channel, &pw_dma_config,
        &pio->txf[ROW_SM], pulse_width_buffer,
        8, false // 8 pulse widths for 8 bits
    );

    // Start both channels
    dma_channel_start(pw_dma_channel);
    dma_channel_start(rgb_dma_channel);
}

void dma_complete(){
    // CLEAR THE INTERRUPT
    dma_irqn_acknowledge_channel(0, rgb_dma_channel);

    // reconfigure both DMA channels
    dma_hw->ch[rgb_dma_channel].al3_read_addr_trig = (uint32_t)front_buffer;
    dma_hw->ch[pw_dma_channel].al3_read_addr_trig = (uint32_t)pulse_width_buffer;

    refresh_count++;
}

void make_bitplanes(uint16_t *in_565, uint8_t *out_888){
    
    for(uint8_t rowsel = 0; rowsel < HEIGHT/2; rowsel++){
        // process a pixel at row 0, then a pixel at row 31, etc.
        for(uint8_t x = 0; x < WIDTH; x++){
            uint32_t pixel_top = rgb565_to_rgb888(in_565[rowsel * WIDTH + x]);
            uint32_t pixel_bottom = rgb565_to_rgb888(in_565[(HEIGHT/2 + rowsel) * WIDTH + x]);
            // Now each pixel is in the binary form 00000000 BBBBBBBB GGGGGGGG RRRRRRRR
            for(uint8_t bit = 0; bit < 8; bit++){
                uint8_t rgb = (pixel_top >> (bit)      & 0b001)
                            | (pixel_top >> (7 + bit)  & 0b010)
                            | (pixel_top >> (14 + bit) & 0b100)
                            
                            | ((pixel_bottom >> (bit)      & 0b001) << 3)
                            | ((pixel_bottom >> (7 + bit)  & 0b010) << 3)
                            | ((pixel_bottom >> (14 + bit) & 0b100) << 3);
                            // Now each pixel is like 00BGRBGR, with 3 LSBs the top pixel

                uint32_t index = (bytes_per_bitplane * bit) + (WIDTH * rowsel + x);
                out_888[index] = rgb;
            }
        }
    }
}

inline uint32_t rgb565_to_rgb888(uint16_t pix) {

    // 00000000 00000000 RRRRRGGG GGGBBBBB -> 00000000 BBBBB000 GGGGGG00 RRRRR000
    // uint8_t r = ((pix & 0xF800) >> 11 << 3);
    // uint8_t g = ((pix & 0x07E0) >> 5 << 2);
    // uint8_t b = ((pix & 0x001F) >> 0 << 3);
    // return (b << 16) | (g << 8) | (r << 0);

    // gamma correct!!!
    uint32_t r_gamma = pix & 0xf800u;
    r_gamma *= r_gamma;
    uint32_t g_gamma = pix & 0x07e0u;
    g_gamma *= g_gamma;
    uint32_t b_gamma = pix & 0x001fu;
    b_gamma *= b_gamma;
    return (b_gamma >> 2 << 16) | (g_gamma >> 14 << 8) | (r_gamma >> 24 << 0);
}