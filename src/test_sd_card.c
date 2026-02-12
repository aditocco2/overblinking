#include <pico/stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "sd_card.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

int main(){
    stdio_init_all();

    int c;
    uint8_t message[1024];
    char *message2 = "test message for block two!!!!";

    while(1){
        c = getchar_timeout_us(0);
        if(c == PICO_ERROR_TIMEOUT){
            continue;
        }
        // printf("%c", c);
        c = (char)c;
        // initialize
        if(c == 'i'){
            if(sd_card_init()){
                printf("SD card init successful\n");
            }
            else{
                printf("SD card init failed\n");
            }
        } 
        // read block at num
        if(isdigit(c)){
            uint8_t n = c - '0';
            if(!sd_card_read_block(n, message, 512)){
                printf("single block read failed\n");
                continue;
            }
            printf("%s", message);
        }
        // read multiblock
        if(c == 'm'){
            if(!sd_card_read_blocks(0, 2, message)){
                printf("multiblock read failed\n");
                continue;
            }
            printf("%s", message);
        }
        // read multiblock and print bytes
        if(c == 'b'){
            if(!sd_card_read_blocks(0, 2, message)){
                printf("multiblock read failed\n");
                continue;
            }
            for(int i = 0; i < 1024; i++){
                if(i % 32 == 0){
                    printf("\n%d\t", i);
                }
                printf("%02x ", message[i]);
            }
        }
        // write data
        if(c == 'w'){
            if(!sd_card_write_block(2, message2, strlen(message2))){
                printf("single block write failed\n");
                continue;
            }
            printf("wrote message to block 2\n");
        }
    }
}
