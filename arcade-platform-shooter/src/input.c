#include "shared.h"

static Input_Context *context;

void input_setup(Input_Context *input_context) {
	context = input_context;

	// Read config file into context.
	FILE *fp = fopen("./config.txt", "rb");
	// Sensible defaults, assume QWERTY.
	if (!fp) {
		context->left = SDL_SCANCODE_A;
		context->right = SDL_SCANCODE_D;
		context->jump = SDL_SCANCODE_W;
		context->shoot = SDL_SCANCODE_K;
	} else {
		fscanf(fp, "left = %hhu\nright = %hhu\njump = %hhu\nshoot = %hhu\n", &context->left, &context->right, &context->jump, &context->shoot);
	}
	fclose(fp);
}
