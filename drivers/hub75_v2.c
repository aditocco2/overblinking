#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
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
static uint32_t data_program_offset;
static uint32_t row_program_offset;

// The back buffer in our case right now is the test gradient from the header file
static uint16_t *back_buffer;
// this one is actually sent to the screen and has RGB888 bitplanes
static uint32_t front_buffer[WIDTH * HEIGHT];

static uint32_t bytes_per_bitplane = WIDTH * HEIGHT/2;

// 
static uint32_t pulse_width_buffer[] = {
    100, 200, 400, 800, 1600, 3200, 6400, 12800
};

void hub75_configure();
void hub75_load_image();
void hub75_push();
void hub75_refresh();
void make_bitplanes(uint16_t *in_565, uint8_t *out_888);
inline uint32_t rgb565_to_rgb888(uint16_t pix);

void main(void){
    stdio_init_all();
    hub75_configure();
    hub75_load_image(test_gradient);
    hub75_push();
    while(1){
        hub75_refresh();
        printf("refresh finished\n");
    }
}

void hub75_configure(){
    // Get the locations (offsets) of the PIO programs in memory
    data_program_offset = pio_add_program(pio, &hub75_data_program);
    row_program_offset = pio_add_program(pio, &hub75_row_program);

    // initialize PIO programs, passing in screen width to the data program
    hub75_data_program_init(pio, DATA_SM, data_program_offset, DATA_BASE_PIN, CLK_PIN, WIDTH);
    hub75_row_program_init(pio, ROW_SM, row_program_offset, ROWSEL_BASE_PIN, ROWSEL_N_PINS, STROBE_PIN);
}

void hub75_load_image(uint16_t * image_pointer){
    back_buffer = image_pointer;
}

void hub75_push(){
    // upscale to RGB888 and make bitplanes in the front buffer
    make_bitplanes(back_buffer, (uint8_t *)front_buffer);
}

void hub75_refresh(){

    int i = 0;

    for(uint8_t bit = 0; bit < 8; bit++){
        pio_sm_put_blocking(pio, ROW_SM, pulse_width_buffer[bit]);
    }
    for(uint32_t data_index = 0; data_index < WIDTH * HEIGHT; data_index++){
        pio_sm_put_blocking(pio, DATA_SM, front_buffer[i]);
        i++;
    }

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
    uint8_t r = ((pix & 0xF800) >> 11 << 3);
    uint8_t g = ((pix & 0x07E0) >> 5 << 2);
    uint8_t b = ((pix & 0x001F) >> 0 << 3);
    return (b << 16) | (g << 8) | (r << 0);
}