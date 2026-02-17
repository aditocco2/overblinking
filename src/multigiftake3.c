/*
Latest version of FreeRTOS implementation
More flickery than the pure Pico SDK implementation, not sure why
*/

#include "FreeRTOS.h"
#include "task.h"
#include "time.h"
#include "FreeRTOSConfig.h"
#include <pico/stdlib.h>
#include <pico/rand.h>
#include <stdio.h>
#include "hub75.h"
#include "sd_card.h"

#define MEDIA_SWITCH_INTERVAL_MS 5000

#define SD_INIT_NOTIF_OFFSET 0
#define DATA_READY_NOTIF_OFFSET 1

void hub75_refresh_task(__unused void *params);
void sd_read_frame_task(__unused void *params);
void sd_switch_media_task(__unused void *params);
void sd_init_task(__unused void *params);

uint8_t control_buffer[512];
uint8_t *image_buffer;
uint32_t media_addr;
uint32_t frame_num, num_frames;
uint16_t frame_time_ms;
uint32_t media_num;

_Bool sd_success, sd_start_switching_media, sd_start_reading_frames;

void main(void){
    stdio_init_all();

    xTaskCreate(hub75_refresh_task, "", 256, NULL, 1, NULL);
    xTaskCreate(sd_read_frame_task, "", 256, NULL, 1, NULL);
    xTaskCreate(sd_switch_media_task, "", 256, NULL, 1, NULL);
    xTaskCreate(sd_init_task, "", 256, NULL, 1, NULL);
    
    vTaskStartScheduler();
}

void hub75_refresh_task(__unused void *params){

    hub75_configure();

    while(1){
        hub75_refresh();
        vTaskDelay(1);
    }
}

void sd_init_task(__unused void *params){

    while(1){
        if(!sd_success){
            vTaskSuspendAll();
            sd_success = sd_card_init();
            xTaskResumeAll();
        }
        vTaskDelay(1);
    }
}

void sd_switch_media_task(__unused void *params){

    TickType_t last_switch_time = xTaskGetTickCount();

    uint8_t num_media;

    // Read the very first portion of SD card memory to get the number of videos
    while(1){
        
        vTaskDelay(1);

        if(sd_success){
            vTaskSuspendAll();
            sd_success = sd_card_read_block(0, control_buffer, 512);
            xTaskResumeAll();
        }
        if(!sd_success){
            vTaskDelay(1);
            continue;
        }

        num_media = (uint32_t)control_buffer[0];
        break;
    }

    // Now we can switch media every N seconds
    while(1){
        // Pick a random media index and load its data (address/length/frametime)
        if(num_media > 1){ 
            uint32_t random_number = get_rand_32() % (num_media - 1);
            if(random_number >= media_num){
                media_num = random_number + 1;
            }
            else{
                media_num = random_number;
            }
        }
        // handle div/0 case
        else{
            media_num = 0;
        }

        uint32_t table_row = media_num + 1;
        uint16_t sector = table_row / (512 / 16); // 16 bytes per row
        uint16_t table_row_index = table_row * 16 % 512;

        vTaskSuspendAll();
        sd_success = sd_card_read_block(sector, control_buffer, 512);
        xTaskResumeAll();
        if(!sd_success){
            vTaskDelay(1);
            continue;
        }

        // table row byte assignment:
        // 0 through 3: sector address
        // 4 through 7: number of frames
        // 8 through 9: frame duration
        // 10 through 15: unused
        media_addr = *(uint32_t *)&control_buffer[table_row_index];
        num_frames = *(uint32_t *)&control_buffer[table_row_index + 4];
        frame_time_ms = *(uint16_t *)&control_buffer[table_row_index + 8];

        frame_num = 0;

        sd_start_reading_frames = true;

        vTaskDelayUntil(&last_switch_time, pdMS_TO_TICKS(MEDIA_SWITCH_INTERVAL_MS));
    }
}

void sd_read_frame_task(__unused void *params){

    image_buffer = (uint8_t *)hub75_get_back_buffer();

    // First wait until the first piece of metadata is loaded
    while(1){
        if(sd_start_reading_frames){
            break;
        }
        vTaskDelay(1);
    }

    TickType_t last_frame_time = xTaskGetTickCount();

    // Then start loading frames into the hub75 buffer
    while(1){
        
        for(uint8_t i = 0; i < 16; i++){
            vTaskSuspendAll();
            sd_success = sd_card_read_block((16 * frame_num + media_addr + i), image_buffer + 512 * i, 512);
            xTaskResumeAll();
            vTaskDelay(1);
        }
    
        if(!sd_success){
            vTaskDelay(1);
            continue;
        }

        frame_num++;
        if(frame_num == num_frames){
            frame_num = 0;
        }

        hub75_push();

        vTaskDelayUntil(&last_frame_time, pdMS_TO_TICKS(frame_time_ms));
    }

}