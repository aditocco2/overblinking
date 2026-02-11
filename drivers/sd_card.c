// Basic SD card driver for SPI
// Adapted from Yaseen Twati's guide:
// https://web.archive.org/web/20250324102712/http://yaseen.ly/writing-data-to-sdcards-without-a-filesystem-spi/

// MAY OR MAY NOT WORK. STILL NEED TO READ DATA

#include <stdbool.h>
#include <stdint.h>
#include "hardware/spi.h"
#include "pico/time.h"

#include <stdio.h>
#include <pico/stdlib.h>

#define MISO 16
#define CS 17
#define SCLK 18
#define MOSI 19

#define BLOCK_SIZE 512
#define TIMEOUT_MS 300

// spi_read_blocking needs a pointer to dummy data so here it is
static uint8_t high = 0xFF;

_Bool sd_card_init();
// _Bool sd_card_write_block(uint8_t *data, uint32_t block_num);
_Bool sd_card_read_block(uint32_t block_addr, uint8_t* buffer, uint16_t buffer_size);

static void send_cmd(uint8_t cmd, uint32_t arg, uint8_t crc);
static uint8_t read_response();
static _Bool wait_card_busy();

static void send_cmd(uint8_t cmd, uint32_t arg, uint8_t crc){

    uint8_t buf[6] = {0x40 | cmd, arg >> 24, arg >> 16, arg >> 8, arg, crc};

    spi_write_blocking(spi0, buf, 6);
}

// keep going until we get a byte whose error MSB isn't high
static uint8_t read_response(){

    uint8_t response = 0xFF;
    absolute_time_t start_time = get_absolute_time();
    while(response >= 0x80){ // MSB high
        spi_read_blocking(spi0, 0xFF, &response, 1);
        absolute_time_t current_time = get_absolute_time();
        if(absolute_time_diff_us(start_time, current_time) >= 1000 * TIMEOUT_MS){
            printf("Timeout\n");
            break;
        }
    }

    return response;
}

// card is busy until we get 0xFF again
// return true on getting 0xFF, return false on timeout
static _Bool wait_card_busy(){

    uint8_t response = 0xFF;
    absolute_time_t start_time = get_absolute_time();
    while(1){
        spi_read_blocking(spi0, 0xFF, &response, 1);
        if(response == 0xFF){
            return true;
        }
        absolute_time_t current_time = get_absolute_time();
        if(absolute_time_diff_us(start_time, current_time) >= 1000 * TIMEOUT_MS){
            return false;
        }
    }
}

// initialize card, assuming V2
_Bool sd_card_init(){

    sleep_us(300);
    
    gpio_init(CS);
    gpio_set_dir(CS, GPIO_OUT);
    gpio_put(CS, 1);

    gpio_set_function(MISO, GPIO_FUNC_SPI);
    gpio_set_function(SCLK, GPIO_FUNC_SPI);
    gpio_set_function(MOSI, GPIO_FUNC_SPI);
    gpio_pull_up(MISO);

    // must be <400KHz during init
    spi_init(spi0, 50000);
    spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    // hold CS high for at least 74 clock cycles to switch the card to native mode
    for(uint8_t i = 0; i < 10; i++){
        spi_write_blocking(spi0, &high, 1);
    }
    gpio_put(CS, 0);

    // send CMD0 to reset into idle state
    send_cmd(0, 0, 0x95);
    if(read_response() != 0x01){
        printf("did not respond to cmd0\n");
        gpio_put(CS, 1);
        return false;
    }

    gpio_put(CS, 1);
    spi_write_blocking(spi0, &high, 1);
    gpio_put(CS, 0);

    // verify V2 with CMD8
    send_cmd(8, 0x01AA, 0x87);
    if(read_response() != 0x01){
        printf("did not respond to cmd8\n");
        gpio_put(CS, 1);
        return false;
    }

    gpio_put(CS, 1);
    spi_write_blocking(spi0, &high, 1);    

    // high capacity mode may take a few tries
    for(uint8_t i = 0; i < 100; i++){

        gpio_put(CS, 0);
        spi_write_blocking(spi0, &high, 1);

        // inform the card next command is an ACMD
        send_cmd(55, 0, 0x65);
        // while(read_response() != 0x01);
        uint8_t response = read_response();

        // I guess we don't actually need the response to be 0x01 :shrug:
        // if(response != 0x01){
        //     printf("did not respond to cmd55 (%d)\n", response);
        //     gpio_put(CS, 1);
        //     return false;
        // }

        // dummy byte???
        spi_write_blocking(spi0, &high, 1);

        // set high capacity mode, if we get 0 it was successful
        send_cmd(41, 0x40000000, 0x77);
        response = read_response();
        printf("cmd41 (%x)\n", response);
        if(response == 0x00){
            break;
        }

        gpio_put(CS, 1);
    }

    gpio_put(CS, 1);

    // extra dummy byte
    spi_write_blocking(spi0, &high, 1);

    return true;
}


_Bool sd_card_read_block(uint32_t block_addr, uint8_t* buffer, uint16_t buffer_size){
    
    gpio_put(CS, 0);
    
    // send CMD17 single read block with the block address at the parameter
    send_cmd(17, block_addr, 0x01);
    if(read_response() != 0x00){
        gpio_put(CS, 1);
        return false;
    }

    uint8_t r_byte;

    for(uint16_t i = 0; i < BLOCK_SIZE; i++){

        spi_read_blocking(spi0, 0xFF, &r_byte, 1);

        if(i < buffer_size){
            buffer[i] = r_byte;
        }
    }
        
    spi_write_blocking(spi0, &high, 1);
    spi_write_blocking(spi0, &high, 1);

    gpio_put(CS, 1);

    spi_write_blocking(spi0, &high, 1);

    return true;

}