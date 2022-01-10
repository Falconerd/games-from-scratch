#include <glad/glad.h>  
#include <SDL2/SDL.h>
#include <stdio.h>
#include "render/render.h"

int main(void) {
    SDL_Window *window = render_init(800, 600);

    SDL_Event e;
    int quit = 0;

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            }
        }

        glClearColor(0, 0.7, 0.9, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        render_quad((vec2){100, 100}, (vec2){100, 100}, (vec2){1, 0, 0.5f, 1});

        SDL_GL_SwapWindow(window);
    }

    return 0;
}