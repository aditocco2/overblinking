#include <pico/stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "sd_card.h"

int main(){
    stdio_init_all();

    uint8_t message[512];

    while(1){
        sleep_ms(1000);
        _Bool success = sd_card_init();
        if(success){
            printf("SD init successful\n");

            success = sd_card_read_block(0, message, 512);
            if(success){
                printf("%s", message);
            }
            else{
                printf("reading failed \n");
            }

        }
        else{
            printf("SD init failed\n");
        }

        
        
    }
}
