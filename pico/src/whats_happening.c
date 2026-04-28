#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <pico/stdlib.h>

#include "hub75.h"
#include "hub75_text.h"
#include "rgb565_colors.h"

// void hexdump(uint8_t *str, uint16_t len);

int main(){
    stdio_init_all();

    hub75_configure();
    hub75_set_brightness(10);



    while("chud"){
        // hub75_set_pixel(31, 31, 0b0000000000000001);
        // hub75_set_pixel(32, 32, 0b0000100000000000);
        hub75_set_pixel(63, 40, 0xFFFF);
        hub75_update();

        int c = getchar_timeout_us(0);
        if(c == 'h'){
            // hexdump((uint8_t *)hub75_get_back_buffer, WIDTH * HEIGHT * 2);
            // hub75_update();
        }
    }
}

// void hexdump(uint8_t *str, uint16_t len){
//     for(uint16_t i = 0; i < len; i++){
//         // every 32 bytes, print a row header
//         if(i % 32 == 0){
//             printf("\n%d\t", i);
//         }
//         // print byte
//         printf("%02x ", str[i]);
//     }
// }