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

static Config_State *config_state;
static SDL_Window *window;
static Input_State *input_state;
static Physics_State *physics_state;
static Time_State *time_state;
static Entity_State *entity_state;

// These could be put elsewhere.
static Entity *player;
static Body *body_player;

static Body_Static *body_a;
static Body_Static *body_b;
static Body_Static *body_c;
static Body_Static *body_d;
static Body_Static *body_e;

	vec2 p0, p1;
	Hit hit;
	AABB test_aabb = {{ 0, 0 }, { 25, 25 }};
	AABB hit_aabb = {{ 0, 0 }, { 25, 25 }};

static void handle_input(bool *quit) {
	if (input_state->quit == KEY_STATE_PRESSED) {
		*quit = true;
	}

	f32 velx = 0;
	f32 vely = body_player->velocity[1];

	if (input_state->right == KEY_STATE_HELD || input_state->right == KEY_STATE_PRESSED) {
		velx += 8200;
	}

	if (input_state->left == KEY_STATE_HELD || input_state->left == KEY_STATE_PRESSED) {
		velx -= 800;
	}

	vely = 0;
	if (input_state->jump == KEY_STATE_HELD || input_state->jump == KEY_STATE_PRESSED) {
		vely = 1500;
	}

	if (input_state->shoot == KEY_STATE_HELD || input_state->shoot == KEY_STATE_PRESSED) {
		vely -= 3000;
	}

	body_player->velocity[0] = velx;
	body_player->velocity[1] = vely;
}

static void render_update_begin() {
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
}

static void render_update_end(SDL_Window *window) {
	SDL_GL_SwapWindow(window);
}

static void game_setup() {
	f32 width = config_state->display_width;
	f32 height = config_state->display_height;
	u32 player_index = entity_create();
	player = &entity_state->entities[player_index];
	player->body_id = physics_body_create((vec2){500, 500}, (vec2){50, 50});
	body_player = array_list_at(physics_state->body_list, player->body_id);
	body_player->is_active = true;

	body_a = array_list_at(physics_state->body_static_list, physics_body_static_create((vec2){width*0.5-25, height-25}, (vec2){width-50, 50}));
	body_b = array_list_at(physics_state->body_static_list, physics_body_static_create((vec2){width-25, height*0.5+25}, (vec2){50, height-50}));
	body_c = array_list_at(physics_state->body_static_list, physics_body_static_create((vec2){width*0.5+25, 25}, (vec2){width-50, 50}));
	body_d = array_list_at(physics_state->body_static_list, physics_body_static_create((vec2){25, height*0.5-25}, (vec2){50, height-50}));

	body_a->is_active = true;
	body_b->is_active = true;
	body_c->is_active = true;
	body_d->is_active = true;
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

	game_setup();

	SDL_Event e;
	bool quit = false;

	while (!quit) {
		time_update();

		// Quit when sending OS quit signal (pressing the X, etc).
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				quit = true;
			}

			if (e.type == SDL_MOUSEBUTTONDOWN) {
				if (e.button.button == SDL_BUTTON_LEFT) {
					p0[0] = input_state->mouse_x;
					p0[1] = config_state->display_height - input_state->mouse_y;
				} else {
					p1[0] = input_state->mouse_x;
					p1[1] = config_state->display_height - input_state->mouse_y;
				}
			}
		}

		input_update(config_state->keybinds);
		handle_input(&quit);
		render_update_begin();
		physics_update(time_state->delta);

		// Rendering stuff goes here
		render_aabb(&body_player->aabb, GREEN);
		render_aabb(&body_a->aabb, WHITE);
		render_aabb(&body_b->aabb, WHITE);
		render_aabb(&body_c->aabb, WHITE);
		render_aabb(&body_d->aabb, WHITE);
		render_line_segment(p0, p1, YELLOW);
		test_aabb.position[0] = p0[0];
		test_aabb.position[1] = p0[1];
		render_aabb(&test_aabb, YELLOW);

		render_text("test", (vec2){200, 200}, WHITE, false);

		AABB x = aabb_sum(body_a->aabb, test_aabb);
		render_aabb(&x, (vec4){1, 1, 1, 0.5});

		render_point(hit.position, GREEN);

		render_aabb(&hit_aabb, GREEN);

		render_update_end(window);
		time_late_update();
	}

	return 0;
}

