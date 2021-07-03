#ifndef src_render_h_INCLUDED
#define src_render_h_INCLUDED

#include "shared.h"

#define SCALE 4
#define WIDTH 256
#define HEIGHT 224

typedef struct render_context {
	GLFWwindow *window;
	m4 projection;
	u32 color_texture;
	u32 shader;
	u32 square_vao;
	u32 square_vbo;
	u32 square_ebo;
	u32 line_vao;
	u32 line_vbo;
} Render_Context;

Render_Context *render_setup(const char *title);
u32 shader_setup();
void render_square(f32 x, f32 y, f32 width, f32 height, vec4 color);
void render_sprite(u32 texture, vec3 position, vec2 size, u8 flipped);
u32 render_create_texture(const char *path);

#endif

