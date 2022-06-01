#ifndef INPUT_H
#define INPUT_H

#include <stdlib.h>
#include <stdint.h>
#include "./types.h"

// There are ways to completely decouple this module from the concept
// of a platforming shooter, for example using an array of Key_State
// which could be defined by the engine user.
// However, that is a lot of over-engineering of a general solution
// that is not necessary - as even updating the input files to add and
// update new keys is not a huge deal.
// Make sure to update the size of the keybinds array in config.h.

typedef enum input_key {
	INPUT_KEY_LEFT,
	INPUT_KEY_RIGHT,
	INPUT_KEY_JUMP,
	INPUT_KEY_SHOOT,
	INPUT_KEY_QUIT
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
	Key_State quit;
	i32 mouse_x;
	i32 mouse_y;
} Input_State;

Input_State *input_init(void);
void input_update(u8 *keybinds);

#endif

