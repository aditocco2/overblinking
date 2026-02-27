#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/sync.h"
#include "hub75.h"
#include "rgb565_colors.h"
#include "hub75_text.h"
#include "images/test_gradient.h"
#include "images/ecd1015.h"

uint32_t refresh_count = 0;
uint8_t brightness = 10;
absolute_time_t ts;

void refresh_cb(){
    refresh_count++;
}

void main(void){
    stdio_init_all();
    hub75_configure();
    hub75_set_refresh_cb(refresh_cb);
    hub75_load_image(test_gradient);
    hub75_update();
    while(1){
        // print refresh count every second
        if (absolute_time_diff_us(ts, get_absolute_time()) > 1000000){
            ts = get_absolute_time();
            printf("Refresh rate: %d Hz\n", refresh_count);
            refresh_count = 0;
        }

        int c = getchar_timeout_us(0);
        if(c == 'w'){
            brightness++;
            hub75_set_brightness(brightness);
            printf("brightness = %d\n", brightness);
        }
        else if (c == 's'){
            brightness--;
            hub75_set_brightness(brightness);
            printf("brightness = %d\n", brightness);
        }
        else if (c == 'm'){
            hub75_set_brightness(255);
            printf("brightness = 255\n");
        }
        else if (c == 'n'){
            hub75_set_brightness(brightness);
            printf("brightness = %d\n", brightness);
        }
        else if (c == 't'){
            hub75_write_large_text("test", 10, 10, ALIGN_LEFT, ALIGN_TOP, RGB565_Yellow);
            hub75_update();
        }
    }
}