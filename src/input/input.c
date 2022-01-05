#include <stdlib.h>
#include <SDL2/SDL.h>
#include "input.h"

static uint32_t KEY_LEFT = SDL_SCANCODE_A;
static uint32_t KEY_RIGHT = SDL_SCANCODE_D;
static uint32_t KEY_JUMP = SDL_SCANCODE_SPACE;
static uint32_t KEY_SHOOT = SDL_SCANCODE_H;

void input_update(Input_State *input_state) {
    const uint8_t *keyboard_state = SDL_GetKeyboardState(NULL);

    // SDL_Scancode a_code = SDL_GetScancodeFromKey(SDL_GetKeyFromName("A"));
    // printf("%d\n", a_code);

    input_state->left = 0;
    input_state->right = 0;

    if (keyboard_state[KEY_LEFT]) {
        input_state->left = 1;
    }

    if (keyboard_state[KEY_RIGHT]) {
        input_state->right = 1;
    }
}