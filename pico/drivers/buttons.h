#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdbool.h>
#include <stdint.h>

_Bool configure_button(uint8_t pin);
_Bool attach_press(uint8_t pin, void (*cb)());
_Bool button_is_up(uint8_t pin);
_Bool button_is_down(uint8_t pin);

#endif