#ifndef RENDER_INTERNAL_H
#define RENDER_INTERNAL_H

#include <SDL2/SDL.h>
#include <linmath.h>

SDL_Window *render_init_window(float width, float height);
void render_init_context(SDL_Window *window);
void render_init_shaders(uint32_t *default_shader, mat4x4 projection, float width, float height);
void render_init_quad(uint32_t *quad_vao, uint32_t *quad_vbo, uint32_t *quad_ebo);
void render_init_color_texture(uint32_t *color_texture);

uint32_t render_shader_create(const char *vert_path, const char *frag_path);

#endif
