#include <pico/stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "sd_card.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/*
i: initialize SD card
0-9: read from that block
m: multiblock read 0 and 1
b: print in bytes
w: write to block 2
c: clear block 2
*/


uint8_t message[1024];
absolute_time_t ts;

int main(){
    stdio_init_all();

    int c;
    char *message2 = "test message for block two!";
    char *blank = "";

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
        // print read data in bytes
        if(c == 'b'){
            for(int i = 0; i < 1024; i++){
                if(i % 32 == 0){
                    printf("\n%d\t", i);
                }
                printf("%02x ", message[i]);
            }
        }
        // write data to block 2
        if(c == 'w'){
            if(!sd_card_write_block(2, message2, strlen(message2))){
                printf("single block write failed\n");
                continue;
            }
            printf("wrote message to block 2\n");
        }
        // clear block 2
        if(c == 'c'){
            if(!sd_card_write_block(2, blank, 1)){
                printf("single block write failed\n");
                continue;
            }
            printf("cleared block 2\n");
        }
    }
}