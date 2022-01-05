#ifndef INPUT_H
#define INPUT_H

#include <stdlib.h>

typedef struct input_state {
    uint8_t left;
    uint8_t right;
} Input_State;

void input_update(Input_State *input_state);

#endif