#include "shared.h"

Input_State input_state = {0};
static Input_State *state = &input_state;

void input_setup() {
	// Read config file into state.
	FILE *fp = fopen("./config.txt", "rb");
	// Sensible defaults, assume QWERTY.
	if (!fp) {
		state->left = SDL_SCANCODE_A;
		state->right = SDL_SCANCODE_D;
		state->jump = SDL_SCANCODE_W;
		state->shoot = SDL_SCANCODE_K;
	} else {
		fscanf(fp, "left = %hhu\nright = %hhu\njump = %hhu\nshoot = %hhu\n", &state->left, &state->right, &state->jump, &state->shoot);
	}
	fclose(fp);
}
