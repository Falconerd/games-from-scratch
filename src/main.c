#include <glad/glad.h>  
#include <SDL2/SDL.h>
#include <stdio.h>
#include "render/render.h"
#include "input/input.h"
#include "config/config.h"

static Input_State input_state = {0};

static vec2 quad_position = {0};

int main(void) {
    Config *config = config_init();
    SDL_Window *window = render_init(800, 600);

    SDL_Event e;
    int quit = 0;

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            }
        }

        input_update(&input_state);

        float quad_movement = 0;

        if (input_state.left) {
            --quad_movement;
        }

        if (input_state.right) {
            ++quad_movement;
        }

        glClearColor(0, 0.7, 0.9, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        vec2_add(quad_position, quad_position, (float[]){quad_movement, 0});

        render_quad(quad_position, (float[]){100, 100}, (float[]){1, 0, 0.5f, 1});

        SDL_GL_SwapWindow(window);
    }

    return 0;
}