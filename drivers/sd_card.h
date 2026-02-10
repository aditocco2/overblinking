#ifndef SD_CARD_H
#define SD_CARD_H

#include <stdbool.h>
#include <stdint.h>

_Bool sd_card_init();
_Bool sd_card_write_block(uint8_t *data, uint32_t block_num);
void sd_card_read_block(uint8_t *data, uint32_t block_num);


#endif