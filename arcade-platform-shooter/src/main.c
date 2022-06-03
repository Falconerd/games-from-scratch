#include <glad/glad.h>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <stdio.h>

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

uint8_t aabb_intersect_aabb(AABB a, AABB b) {
    for (int i = 0; i < 2; ++i) {
        if (a.position[i] + a.half_size[i] < b.position[i] - b.half_size[i] || a.position[i] - a.half_size[i] > b.position[i] + b.half_size[i]) {
            return 0;
        }
    }
    return 1;
}

uint8_t aabb_intersect_aabb_moving(AABB a, AABB b, vec2 va, vec2 vb, float *tfirst, float *tlast) {
    if (aabb_intersect_aabb(a, b)) {
        *tfirst = *tlast = 0.f;
        return 1;
    }

    vec2 v;
    vec2_sub(v, vb, va);

    *tfirst = 0.f;
    *tlast = 1.f;

    vec2 amin, amax, bmin, bmax;
    vec2_sub(amin, a.position, a.half_size);
    vec2_sub(bmin, b.position, b.half_size);
    vec2_add(amax, a.position, a.half_size);
    vec2_add(bmax, b.position, b.half_size);

    for (uint8_t i = 0; i < 2; ++i) {
        if (v[i] < 0.f) {
            if (bmax[i] < amin[i]) return 0;
            if (amax[i] < bmin[i]) *tfirst = fmaxf((amax[i] - bmin[i]) / v[i], *tfirst);
            if (bmax[i] > amin[i]) *tlast = fminf((amin[i] - bmax[i]) / v[i], *tlast);
        }

        if (v[i] > 0.f) {
            if (bmin[i] > amax[i]) return 0;
            if (bmax[i] < amin[i]) *tfirst = fmaxf((amin[i] - bmax[i]) / v[i], *tfirst);
            if (amax[i] > bmin[i]) *tlast = fminf((amax[i] - bmin[i]) / v[i], *tlast);
        }

        if (*tfirst > *tlast) return 0;
    }

    return 1;
}

static void render_update_begin() {
	glClearColor(0, 0.2, 0.3, 1);
	glClear(GL_COLOR_BUFFER_BIT);
}

static void render_update_end(SDL_Window *window) {
	SDL_GL_SwapWindow(window);
}

int main(int argc, char *argv[]) {
	// Fix buffer not being flushed on print, for "print debugging".
	setvbuf(stdout, NULL, _IONBF, 0);

	config_state = config_init();
	window = render_init(config_state->display_width, config_state->display_height);
	input_state = input_init();
	physics_state = physics_init();
	time_state = time_init(config_state->framerate);
	entity_state = entity_init();

	SDL_Event e;
	bool quit = false;

	AABB a = {{250, 50}, {25, 25}};
	AABB b = {{450, 850}, {25, 25}};
	AABB c = {{350, 150}, {200, 25}};

	vec2 a_vel = {0, 300};
	vec2 a_next;
	vec2_add(a_next, a.position, a_vel);
	AABB a_next_aabb;
	memcpy(&a_next_aabb, &a, sizeof(a));
	memcpy(&a_next_aabb.position, a_next, sizeof(vec2));

	vec2 b_vel = {10, -500};
	vec2 b_next;
	vec2_add(b_next, b.position, b_vel);
	AABB b_next_aabb;
	memcpy(&b_next_aabb, &b, sizeof(b));
	memcpy(&b_next_aabb.position, b_next, sizeof(vec2));

	f32 tfirst;
	f32 tlast;

	while (!quit) {
		time_update();

		// Quit when sending OS quit signal (pressing the X, etc).
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				quit = true;
			}
		}

		input_update(config_state->keybinds);
	        SDL_GetMouseState(&input_state->mouse_x, &input_state->mouse_y);

		render_update_begin();

		render_aabb(&a, WHITE);
		render_aabb(&b, WHITE);
		render_aabb(&c, WHITE);

		uint8_t collided = aabb_intersect_aabb_moving(a, c, a_vel, (vec2){0, 0}, &tfirst, &tlast);

		render_aabb(&a_next_aabb, YELLOW);
		render_aabb(&b_next_aabb, YELLOW);

		if (collided) {
			AABB collision;
			collision.position[0] = a.position[0] + a_vel[0] * tfirst;
			collision.position[1] = a.position[1] + a_vel[1] * tfirst;
			collision.half_size[0] = a.half_size[0];
			collision.half_size[1] = a.half_size[1];
			render_aabb(&collision, GREEN);
		}

		collided = aabb_intersect_aabb(b, c, b_vel, (vec2){0, 0}, &tfirst, &tlast);

		if (collided) {
			AABB collision;
			collision.position[0] = b.position[0] + b_vel[0] * tfirst;
			collision.position[1] = b.position[1] + b_vel[1] * tfirst;
			collision.half_size[0] = b.half_size[0];
			collision.half_size[1] = b.half_size[1];
			render_aabb(&collision, GREEN);
		}

		render_update_end(window);
		time_late_update();
	}

	return 0;
}

