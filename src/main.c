#include <glad/glad.h>
#include <stdio.h>
#include "state.h"
#include "render/render.h"

#define WIDTH 800
#define HEIGHT 600

int main(void) {
    State state = {0};

    render_init(&state.window, WIDTH, HEIGHT);

    printf("Hello there!\n");

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

        render_quad((float[]){0, 0}, (float[]){100, 100}, (float[]){1, 1, 1, 1});

        SDL_GL_SwapWindow(state.window);
    }

    return 0;
}