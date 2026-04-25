#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

void fill_array(uint32_t *arr, uint32_t len){
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

int main(){
    srand(146);
    uint32_t arr[20];
    fill_array(arr, 20);
    shuffle_array(arr, 20);

    printf("shuffled array: [ ");
    for(uint32_t i = 0; i < 20; i++){
        printf("%d ", arr[i]);
    }
    printf("]\n");
}

