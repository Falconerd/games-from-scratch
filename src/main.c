#include <glad/glad.h>  
#include <SDL2/SDL.h>
#include <stdio.h>
#include "render/render.h"
#include "input/input.h"
#include "config/config.h"

static vec2 quad_position = {0};

int main(void) {
    Config_State *config_state = config_init();
    Input_State *input_state = input_init();
    SDL_Window *window = render_init(800, 600);

    SDL_Event e;
    int quit = 0;

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            }
        }

        input_update(config_state->keybinds);

        float quad_movement = 0;

        if (input_state->left) {
            --quad_movement;
        }

        if (input_state->right) {
            ++quad_movement;
        }

        vec2_add(quad_position, quad_position, (vec2){quad_movement, 0});

        glClearColor(0, 0.7, 0.9, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        render_quad(quad_position, (vec2){100, 100}, (vec2){1, 0, 0.5f, 1});

        SDL_GL_SwapWindow(window);
    }

    return 0;
}