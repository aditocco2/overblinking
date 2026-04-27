#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define MAX_MEDIA 5

typedef struct{
    uint16_t switch_interval_s;
    _Bool use_static_mode;
    _Bool randomize;

    // mode_t current_mode;

    // media_type_t current_media_type;
    uint32_t media_address;
    uint16_t frame_duration_ms;
    uint32_t current_media_index;
    uint32_t current_frame_num;
    uint32_t num_frames_in_current_media;
    uint32_t pool_size;
} player_data_t;

void main_fsm();
void slideshow_media_index_fsm(player_data_t *ts, _Bool reset);
void fill_array_sequentially(uint32_t *arr, uint32_t len);
void shuffle_array(uint32_t *arr, uint32_t len);

int main(){
    player_data_t ts;
    ts.pool_size = 5;
    _Bool reset;
    char c;

    while(1){
        scanf("%c\n", &c);
        reset = (c == 'r');
        slideshow_media_index_fsm(&ts, reset);
    }
}

void slideshow_media_index_fsm(player_data_t *ts, _Bool reset){
    static enum {INIT, SHUFFLE, TRAVERSE} state;

    static uint32_t media_index_array[MAX_MEDIA];
    static uint32_t tag = 0;

    if(reset){
        state = INIT;
    }

    switch(state){
        case INIT:
            fill_array_sequentially(media_index_array, ts->pool_size);
            tag = 0;
            if(ts->randomize){
                state = SHUFFLE;
            }
            else{
                state = TRAVERSE;
            }
            break;
        case SHUFFLE:
            shuffle_array(media_index_array, ts->pool_size);
            tag = 0;
            state = TRAVERSE;
            break;
        case TRAVERSE:
            tag++;
            if(ts->randomize && tag >= ts->pool_size - 1){
                state = SHUFFLE;
            }
            else if(tag >= ts->pool_size){
                tag = 0;
            }
            break;
    }

    ts->current_media_index = media_index_array[tag];

    printf("[ ");
    for(uint32_t i = 0; i < ts->pool_size; i++){
        printf("%d ", media_index_array[i]);
    }
    printf("] -> %d\n", ts->current_media_index);
}

void fill_array_sequentially(uint32_t *arr, uint32_t len){
    for(uint32_t i = 0; i < len; i++){
        arr[i] = i;
    }
}

void shuffle_array(uint32_t *arr, uint32_t len){
    for(uint32_t i = len - 1; i > 0; i--){
        uint32_t j = rand() % (i + 1);

        uint32_t temp = arr[j];
        arr[j] = arr[i];
        arr[i] = temp;
    }
}