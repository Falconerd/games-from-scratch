#include <glad/glad.h>
#include <SDL2/SDL.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "../render.h"
#include "../../deps/lib/linmath.h"
#include "../types.h"
#include "render_internal.h"

static mat4x4 projection;
static u32 shader_default;
static u32 shader_text;
static u32 vao_text, vbo_text;
static u32 vao_quad, vbo_quad, ebo_quad;
static u32 vao_line, vbo_line;
static u32 texture_color;

static Font font_array[2] = {
	{ .path = "./assets/8-BIT_WONDER.TTF", .size = 24 },
	{ .path = "./assets/PxPlus_IBM_VGA_8x16.ttf", .size = 16 },
};

//TODO SCALE
SDL_Window *render_init(f32 width, f32 height) {
	SDL_Window *window = render_init_window(width, height);

	render_init_context(window);
	render_init_shaders(&shader_default, projection, width, height);
	render_init_quad(&vao_quad, &vbo_quad, &ebo_quad);
	render_init_line(&vao_line, &vbo_line);
	render_init_color_texture(&texture_color);
	FT_Library ft = render_init_text_begin(&shader_text, &vao_text, &vbo_text, width, height, 1);
	render_init_font(&font_array[0], ft);
	render_init_font(&font_array[1], ft);
	render_init_text_end(ft);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	return window;
}

void render_quad(vec2 pos, vec2 size, vec4 color) {
	glUseProgram(shader_default);

	mat4x4 model;
	mat4x4_identity(model);

	mat4x4_translate(model, pos[0] + size[0] * 0.5, pos[1] + size[1] * 0.5, 0);
	mat4x4_scale_aniso(model, model, size[0], size[1], 1);

	glUniformMatrix4fv(glGetUniformLocation(shader_default, "model"), 1, GL_FALSE, &model[0][0]);
	glUniform4fv(glGetUniformLocation(shader_default, "color"), 1, color);

	glBindVertexArray(vao_quad);
	{
		glBindTexture(GL_TEXTURE_2D, texture_color);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
	}
	glBindVertexArray(0);
}

void render_aabb(void *aabb, vec4 color) {
	glUseProgram(shader_default);

	f32 *pos = (f32*)aabb;
	f32 *size = pos+2;

	vec2 a = { pos[0] - size[0], pos[1] - size[1] };
	vec2 b = { pos[0] + size[0], pos[1] - size[1] };
	vec2 c = { pos[0] + size[0], pos[1] + size[1] };
	vec2 d = { pos[0] - size[0], pos[1] + size[1] };

	render_line_segment(a, b, color);
	render_line_segment(b, c, color);
	render_line_segment(c, d, color);
	render_line_segment(d, a, color);
}

void render_point(vec2 point, vec4 color) {
	render_quad((vec2){point[0] - 1, point[1] - 1}, (vec2){2, 2}, color);
}

void render_line_segment(vec2 start, vec2 end, vec4 color) {
	glUseProgram(shader_default);

	f32 x = end[0] - start[0];
	f32 y = end[1] - start[1];
	f32 line[6] = {0, 0, 0, x, y, 0};
	mat4x4 model;
	mat4x4_identity(model);

	mat4x4_translate(model, start[0], start[1], 0);

	glUniformMatrix4fv(glGetUniformLocation(shader_default, "model"), 1, GL_FALSE, &model[0][0]);
	glUniform4fv(glGetUniformLocation(shader_default, "color"), 1, color);

	glBindTexture(GL_TEXTURE_2D, texture_color);
	glBindVertexArray(vao_line);
	{
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_line);
		glBufferSubData(GL_ARRAY_BUFFER, 0, (sizeof line), &line);
		glDrawArrays(GL_LINES, 0, 2);
		glDisableVertexAttribArray(0);
	}
	glBindVertexArray(0);
}

void render_text(const char *text, vec2 pos, vec4 color, bool is_centered, u32 font_id) {
	glUseProgram(shader_text);
	glUniform4fv(glGetUniformLocation(shader_text, "color"), 1, color);
	glActiveTexture(GL_TEXTURE0);

	Font *font = &font_array[font_id];

	glBindVertexArray(vao_text);

	// TODO
	//vec2_scale(pos, pos, SCALE);
	f32 x = pos[0];
	f32 y = pos[1];

	if (is_centered) {
		f32 width = 0;

		for (const char *p = text; *p; ++p) {
			width += font->character_data_array[(u32)*p].width;
		}

		x -= width * 0.5;
	}

	for (const char *p = text; *p; ++p) {
		Character_Data cd = font->character_data_array[(u32)*p];

		f32 x2 = x + cd.left;
		f32 y2 = y - (cd.height - cd.top);
		f32 w = cd.width;
		f32 h = cd.height;

		f32 vertices[6][4] = {
			{x2, y2 + h, 0, 0},
			{x2, y2, 0, 1},
			{x2 + w, y2, 1, 1},

			{x2, y2 + h, 0, 0},
			{x2 + w, y2, 1, 1},
			{x2 + w, y2 + h, 1, 0}
		};

		glBindTexture(GL_TEXTURE_2D, cd.texture);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_text);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof vertices, vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		x += (cd.advance_x / 64);
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

