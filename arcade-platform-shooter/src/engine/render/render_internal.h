#ifndef RENDER_INTERNAL_H
#define RENDER_INTERNAL_H

#include <SDL2/SDL.h>
#include <freetype/freetype.h>
#include "../../deps/lib/linmath.h"
#include "../types.h"
#include "../render.h"

SDL_Window *render_init_window(f32 width, f32 height);
void render_init_context(SDL_Window *window);
void render_init_shaders(u32 *default_shader, mat4x4 projection, f32 width, f32 height);
void render_init_quad(u32 *quad_vao, u32 *quad_vbo, u32 *quad_ebo);
void render_init_line(u32 *line_vao, u32 *line_vbo);
void render_init_color_texture(u32 *color_texture);
FT_Library render_init_text_begin(u32 *shader, u32 *vao, u32 *vbo, f32 width, f32 height, f32 scale);
void render_init_text_end(FT_Library ft);
void render_init_font(Font *font, FT_Library ft);

u32 render_shader_create(const char *vert_path, const char *frag_path);

#endif

