#include <stdio.h>
#include <stdlib.h>
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

void print_refresh_rate();
void print_brightness();
void refresh_cb();

void main(void){
    stdio_init_all();
    hub75_configure();
    hub75_set_refresh_cb(refresh_cb);
    hub75_set_brightness(brightness);
    hub75_load_image(test_gradient);
    hub75_update();
    while(1){
        // print refresh count every second
        if (absolute_time_diff_us(ts, get_absolute_time()) > 1000000){
            ts = get_absolute_time();
            print_refresh_rate();
            refresh_count = 0;
        }

        int c = getchar_timeout_us(0);
        if(c == 'w'){
            brightness++;
            hub75_set_brightness(brightness);
            print_brightness();
        }
        else if (c == 's'){
            brightness--;
            hub75_set_brightness(brightness);
            print_brightness();
        }
        else if (c == 'm'){
            hub75_set_brightness(255);
            print_brightness();
        }
        else if (c == 'n'){
            hub75_set_brightness(brightness);
            print_brightness();
        }
        else if (c == 't'){
            hub75_write_large_text("big test", 0, 10, ALIGN_LEFT, ALIGN_TOP, RGB565_Yellow);
            hub75_update();
        }
    }
}

void refresh_cb(){
    refresh_count++;
}

void print_brightness(){
    char bright[4];
    hub75_load_image(test_gradient);
    hub75_write_small_text("Brightness: ", 2, 54, ALIGN_LEFT, ALIGN_TOP, RGB565_Red);
    hub75_write_small_text(itoa(brightness, bright, 10), 2 + 12 * 4, 54, ALIGN_LEFT, ALIGN_TOP, RGB565_Red);
    hub75_update(); 
}

void print_refresh_rate(){
    char refresh[4];
    hub75_load_image(test_gradient);
    hub75_write_small_text("Refresh: ", 2, 54, ALIGN_LEFT, ALIGN_TOP, RGB565_Red);
    hub75_write_small_text(itoa(refresh_count, refresh, 10), 2 + 9 * 4, 54, ALIGN_LEFT, ALIGN_TOP, RGB565_Red);
    hub75_update(); 
}