#include <stdlib.h>
#include <SDL2/SDL.h>
#include "../input.h"

static Input_State input_state = {0};

Input_State *input_init(void) {
	return &input_state;
}

static void update_key_state(u8 current_state, Key_State *key_state) {
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

void input_update(u8 *keybinds) {
	SDL_GetMouseState(&input_state.mouse_x, &input_state.mouse_y);

	const u8 *keyboard_state = SDL_GetKeyboardState(NULL);

	input_state.jump_last_frame = input_state.jump;

	update_key_state(keyboard_state[keybinds[INPUT_KEY_LEFT]], &input_state.left);
	update_key_state(keyboard_state[keybinds[INPUT_KEY_RIGHT]], &input_state.right);
	update_key_state(keyboard_state[keybinds[INPUT_KEY_JUMP]], &input_state.jump);
	update_key_state(keyboard_state[keybinds[INPUT_KEY_QUIT]], &input_state.quit);
}

