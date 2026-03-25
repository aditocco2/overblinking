#ifndef PLAYER_H
#define PLAYER_H

#include <stdint.h>

_Bool player_init();
uint32_t player_get_num_media();
_Bool player_load_media(uint32_t index);
void player_play();
void player_pause();
_Bool player_update();

#endif