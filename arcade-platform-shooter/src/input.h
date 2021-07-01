#ifndef src_input_h_INCLUDED
#define src_input_h_INCLUDED

typedef struct input_context {
	u8 player_input;
} Input_Context;

void input_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

Input_Context *input_setup();

#endif

