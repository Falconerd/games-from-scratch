#ifndef RENDER_INTERNAL_H
#define RENDER_INTERNAL_H

#include <SDL2/SDL.h>
#include <freetype/freetype.h>
#include "../../deps/lib/linmath.h"
#include "../types.h"

typedef struct character_data {
	u32 texture;
	u32 advance_x;
	u32 advance_y;
	i32 width;
	i32 height;
	i32 left;
	i32 top;
} Character_Data;

SDL_Window *render_init_window(f32 width, f32 height);
void render_init_context(SDL_Window *window);
void render_init_shaders(u32 *default_shader, mat4x4 projection, f32 width, f32 height);
void render_init_quad(u32 *quad_vao, u32 *quad_vbo, u32 *quad_ebo);
void render_init_line(u32 *line_vao, u32 *line_vbo);
void render_init_color_texture(u32 *color_texture);
void render_init_text(FT_GlyphSlot *g, FT_Face *face, Character_Data *character_data_array, u32 *shader, u32 *vao, u32 *vbo, f32 width, f32 height, f32 scale);

u32 render_shader_create(const char *vert_path, const char *frag_path);

#endif

