#include <pico/stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "sd_card.h"

int main(){
    stdio_init_all();
    

    while(1){
        sleep_ms(1000);
        _Bool success = sd_card_init();
        if(success){
            printf("SD init successful\n");
        }
        else{
            printf("SD init failed\n");
        }
        
    }
}
