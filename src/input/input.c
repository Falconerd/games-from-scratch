#include <stdlib.h>
#include <SDL2/SDL.h>
#include "input.h"

static Input_State input_state = {0};

Input_State *input_init(void) {
    return &input_state;
}

void input_update(uint8_t *keybinds) {
    const uint8_t *keyboard_state = SDL_GetKeyboardState(NULL);

    input_state.left = 0;
    input_state.right = 0;

    if (keyboard_state[keybinds[INPUT_KEY_LEFT]]) {
        input_state.left = 1;
    }

    if (keyboard_state[keybinds[INPUT_KEY_RIGHT]]) {
        input_state.right = 1;
    }
}
