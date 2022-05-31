#ifndef INPUT_H
#define INPUT_H

#include <stdlib.h>
#include <stdint.h>

typedef enum input_key {
	INPUT_KEY_LEFT,
	INPUT_KEY_RIGHT,
	INPUT_KEY_JUMP,
	INPUT_KEY_SHOOT
} Input_Key;

typedef enum key_state {
	KEY_STATE_UNPRESSED,
	KEY_STATE_PRESSED,
	KEY_STATE_HELD
} Key_State;

typedef struct input_state {
	Key_State left;
	Key_State right;
	Key_State jump;
	Key_State jump_last_frame;
	int mouse_x;
	int mouse_y;
} Input_State;

Input_State *input_init(void);
void input_update(uint8_t *keybinds);

#endif

