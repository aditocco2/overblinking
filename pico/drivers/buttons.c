#include "pico/stdlib.h"
#include "pico/time.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define MAX_BUTTONS 5
#define UPDATE_TIME_MS 10

static uint8_t num_configured_buttons = 0;

static uint8_t button_pins[MAX_BUTTONS];
static _Bool button_states_current[MAX_BUTTONS];
static _Bool button_states_previous[MAX_BUTTONS];
static void (*press_cbs[MAX_BUTTONS])();

_Bool configure_button(uint8_t pin);
_Bool attach_press(uint8_t pin, void (*cb)());
_Bool button_is_up(uint8_t pin);
_Bool button_is_down(uint8_t pin);

static void update_button_states();
static _Bool timer_cb(__unused struct repeating_timer *t);

_Bool configure_button(uint8_t pin){
    
    if(num_configured_buttons >= MAX_BUTTONS){
        return false;
    }
    
    gpio_init(pin);
    gpio_pull_up(pin);
    gpio_set_dir(pin, GPIO_IN);

    button_pins[num_configured_buttons] = pin;

    static struct repeating_timer timer;

    // Add the repeating timer if this is the first configure
    if(num_configured_buttons == 0){    
        add_repeating_timer_ms(-1 * UPDATE_TIME_MS, timer_cb, NULL, &timer);
    }

    num_configured_buttons++;
    return true;
}

_Bool attach_press(uint8_t pin, void (*cb)()){
    // search for the specified pin in the pins array and set the callback
    for(uint8_t i = 0; i < num_configured_buttons; i++){
        if(button_pins[i] == pin){
            press_cbs[i] = cb;
            return true;
        }
    }
    return false;
}

static _Bool timer_cb(__unused struct repeating_timer *t){
    update_button_states();
    // If any button is pressed, run its corresponding callback
    for(uint8_t i = 0; i < num_configured_buttons; i++){
        if(button_states_current[i] && !button_states_previous[i]){
            press_cbs[i]();
        }
    }
    return true;
}

static void update_button_states(){
    for(uint8_t i = 0; i < num_configured_buttons; i++){
        button_states_previous[i] = button_states_current[i];
        button_states_current[i] = !gpio_get(button_pins[i]);
    }
}

_Bool button_is_up(uint8_t pin){
    return !button_is_down(pin);
}

_Bool button_is_down(uint8_t pin){
    // search for the specified pin in the pins array and return its corresponding state
    for(uint8_t i = 0; i < num_configured_buttons; i++){
        if(button_pins[i] == pin){
            return button_states_current[pin];
        }
    }
    return false;
}