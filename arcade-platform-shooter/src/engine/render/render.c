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

static Character_Data character_data_array[128];

static FT_Face face;
static FT_GlyphSlot g;


SDL_Window *render_init(f32 width, f32 height) {
	SDL_Window *window = render_init_window(width, height);

	render_init_context(window);
	render_init_shaders(&shader_default, projection, width, height);
	render_init_quad(&vao_quad, &vbo_quad, &ebo_quad);
	render_init_line(&vao_line, &vbo_line);
	render_init_color_texture(&texture_color);
	render_init_text(&g, &face, character_data_array, &vao_text, &vbo_text);

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
	vec2 q;
	vec2 r;

	vec2_sub(q, pos, size);
	vec2_scale(r, size, 2);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	render_quad(q, r, color);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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

void render_text(const char *text, vec2 pos, vec4 color, bool is_centered) {
	glUseProgram(shader_text);
	glUniform4fv(glGetUniformLocation(shader_text, "color"), 1, color);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(vao_text);

	// TODO
	//vec2_scale(pos, pos, SCALE);
	f32 x = pos[0];
	f32 y = pos[1];

	if (is_centered) {
		f32 width = 0;

		for (const char *p = text; *p; ++p) {
			width += character_data_array[(u32)*p].width;
		}

		x -= width * 0.5;
	}

	for (const char *p = text; *p; ++p) {
		Character_Data cd = character_data_array[(u32)*p];

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

