#ifndef RENDER_INTERNAL_H
#define RENDER_INTERNAL_H

#include <SDL2/SDL.h>
#include "../../deps/lib/linmath.h"

void render_init_window(SDL_Window **window, float width, float height);
void render_init_context(SDL_Window **window);
void render_init_shaders(uint32_t *default_shader, uint32_t *text_shader, uint32_t *circle_shader, mat4x4 projection, float width, float height);
void render_init_quad();

void render_init_color_texture(uint32_t *color_texture);
uint32_t render_shader_create(const char *vert_path, const char *frag_path);
void render_texture_setup(uint32_t texture_id);

#endif