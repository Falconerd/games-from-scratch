#include <glad/glad.h>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <stdio.h>

#include <assert.h>

#include "engine/array_list.h"
#include "engine/render.h"
#include "engine/input.h"
#include "engine/config.h"
#include "engine/util.h"
#include "engine/physics.h"
#include "engine/time.h"
#include "engine/entity.h"
#include "game.h"

static Config_State *config_state;
static SDL_Window *window;
static Input_State *input_state;
static Physics_State *physics_state;
static Time_State *time_state;
static Entity_State *entity_state;

static void render_update_begin() {
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
}

static void render_update_end(SDL_Window *window) {
	SDL_GL_SwapWindow(window);
}

int main(int argc, char *argv[]) {
	// Fix buffer in Windows not being flushed on print, for "print debugging".
	setvbuf(stdout, NULL, _IONBF, 0);

	config_state = config_init();
	window = render_init(config_state->display_width, config_state->display_height);
	input_state = input_init();
	physics_state = physics_init();
	time_state = time_init(config_state->framerate);
	entity_state = entity_init();

	game_setup(config_state, physics_state, entity_state);

	SDL_Event e;
	bool quit = false;

	char fps[10] = {0};

	while (!quit) {
		time_update();

		// Quit when sending OS quit signal (pressing the X, etc).
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				quit = true;
			}
		}

		input_update(config_state->keybinds);
		game_handle_input(input_state, &quit);

#if DEBUG
		render_update_begin();
		physics_update(time_state->delta);
#else
		physics_update(time_state->delta);
		render_update_begin();
#endif

		game_render();

		sprintf(fps, "FPS: %u", time_state->frame_rate);
		render_text(fps, (vec2){8, config_state->display_height - 16 }, WHITE, false, 1);

		render_update_end(window);
		time_late_update();
	}

	return 0;
}

