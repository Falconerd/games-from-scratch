#include <stdlib.h>
#include <SDL2/SDL.h>
#include "input.h"

static uint8_t keybinds[4] = {0};

void input_update(Input_State *input_state) {
    const uint8_t *keyboard_state = SDL_GetKeyboardState(NULL);

    input_state->left = 0;
    input_state->right = 0;

    if (keyboard_state[keybinds[INPUT_KEY_LEFT]]) {
        input_state->left = 1;
    }

    if (keyboard_state[keybinds[INPUT_KEY_RIGHT]]) {
        input_state->right = 1;
    }
}

void input_key_bind(Input_Key key, const char *key_name) {
    SDL_Scancode scan_code = SDL_GetScancodeFromName(key_name);
    keybinds[key] = scan_code;
}