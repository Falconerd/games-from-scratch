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

        glClearColor(0.f, 0.2f, 0.3f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        render_quad((vec2){100.f, 100.f}, (vec2){100.f, 100.f}, (vec4){1.f, 0.f, 0.5f, 1.f});

        SDL_GL_SwapWindow(window);
    }

    return 0;
}
