#ifndef SD_CARD_H
#define SD_CARD_H

#include <stdbool.h>
#include <stdint.h>

_Bool sd_card_init();
_Bool sd_card_write_block(uint8_t *data, uint32_t block_num);
_Bool sd_card_read_block(uint32_t block_addr, uint8_t* buffer, uint16_t buffer_size);


#endif