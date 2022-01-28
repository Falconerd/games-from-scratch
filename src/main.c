#include <glad/glad.h>  
#include <SDL2/SDL.h>
#include <stdio.h>
#include "render/render.h"
#include "input/input.h"
#include "config/config.h"
#include "util/util.h"

typedef struct aabb {
    vec2 position;
    vec2 half_size;
} AABB;

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

int main(void) {
    Config_State *config_state = config_init();
    SDL_Window *window = render_init(config_state->display_width, config_state->display_height);

    AABB start_aabb = {{0.f, 0.f}, {50.f, 50.f}};
    AABB end_aabb = {{0.f, 0.f}, {50.f, 50.f}};
    AABB obstacle_aabb = {{config_state->display_width * 0.5f, config_state->display_height * 0.5f}, {20.f, 100.f}};
    vec2 vel;

    SDL_Event e;
    int quit = 0;

    int x, y;

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            }
            if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    start_aabb.position[0] = (float)x;
                    start_aabb.position[1] = config_state->display_height - (float)y;
                } else {
                    end_aabb.position[0] = (float)x;
                    end_aabb.position[1] = config_state->display_height - (float)y;
                }

                vec2_sub(vel, end_aabb.position, start_aabb.position);
            }
        }

        SDL_GetMouseState(&x, &y);

        glClearColor(0.f, 0.2f, 0.3f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        render_aabb(&start_aabb, GREEN);
        render_aabb(&end_aabb, PINK);
        render_aabb(&obstacle_aabb, WHITE);

        float tfirst, tlast;
        if (aabb_intersect_aabb_moving(start_aabb, obstacle_aabb, vel, (vec2){0.f, 0.f}, &tfirst, &tlast)) {
            AABB collision_aabb;
            collision_aabb.position[0] = start_aabb.position[0] + vel[0] * tfirst;
            collision_aabb.position[1] = start_aabb.position[1] + vel[1] * tfirst;
            collision_aabb.half_size[0] = start_aabb.half_size[0];
            collision_aabb.half_size[1] = start_aabb.half_size[1];

            render_aabb(&collision_aabb, YELLOW);
        }

        render_line_segment(start_aabb.position, end_aabb.position, WHITE);

        SDL_GL_SwapWindow(window);
    }

    return 0;
}
