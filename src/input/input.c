#include <stdlib.h>
#include <SDL2/SDL.h>
#include "input.h"

static Input_State input_state = {0};

Input_State *input_init(void) {
    return &input_state;
}

static void update_key_state(uint8_t current_state, Key_State *key_state) {
    // If current key is held (from SDL's KeyboardState).
    if (current_state) {
        // Since KEY_STATE_UNPRESSED is the first enum, it evaluates to 0.
        // Using that, the key must have been pressed last frame or earlier.
        // Else, it was pressed this frame.
        if (*key_state > 0) {
            *key_state = KEY_STATE_HELD;
        } else {
            *key_state = KEY_STATE_PRESSED;
        }
    } else {
        *key_state = KEY_STATE_UNPRESSED;
    }
}

void input_update(uint8_t *keybinds) {
    const uint8_t *keyboard_state = SDL_GetKeyboardState(NULL);

    input_state.jump_last_frame = input_state.jump;

    update_key_state(keyboard_state[keybinds[INPUT_KEY_LEFT]], &input_state.left);
    update_key_state(keyboard_state[keybinds[INPUT_KEY_RIGHT]], &input_state.right);
    update_key_state(keyboard_state[keybinds[INPUT_KEY_JUMP]], &input_state.jump);
}
