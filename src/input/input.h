#ifndef INPUT_H
#define INPUT_H

#include <stdlib.h>

typedef enum input_key {
    INPUT_KEY_LEFT,
    INPUT_KEY_RIGHT,
    INPUT_KEY_JUMP,
    INPUT_KEY_SHOOT
} Input_Key;

typedef struct input_state {
    uint8_t left;
    uint8_t right;
} Input_State;

void input_update(Input_State *input_state);
void input_key_bind(Input_Key key, const char *key_name);

#endif