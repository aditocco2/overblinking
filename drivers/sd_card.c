#include <stdbool.h>
#include <stdint.h>
#include "hardware/spi.h"

#define BLOCK_SIZE 512

#define MISO_PIN 16
#define MOSI_PIN 19
#define SCLK_PIN 18
#define CS_PIN 17

_Bool sd_card_init();
_Bool sd_card_write_block(uint8_t *data, uint32_t block_num);
void sd_card_read_block(uint8_t *data, uint32_t block_num);


