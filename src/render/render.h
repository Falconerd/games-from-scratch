#ifndef RENDER_H
#define RENDER_H

#include <SDL2/SDL.h>
#include "../../deps/lib/linmath.h"

void render_init(SDL_Window **window, float width, float height);
void render_quad(vec2 pos, vec2 size, vec4 color);

#endif