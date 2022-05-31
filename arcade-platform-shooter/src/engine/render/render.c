#include <glad/gl.h>
#include <SDL2/SDL.h>
#include "../../deps/lib/linmath.h"
#include "render_internal.h"

static u32 default_shader;
static mat4x4 projection;
static u32 quad_vao, quad_vbo, quad_ebo;
static u32 line_vao, line_vbo;
static u32 color_texture;

SDL_Window *render_init(f32 width, f32 height) {
	SDL_Window *window = render_init_window(width, height);

	render_init_context(window);
	render_init_shaders(&default_shader, projection, width, height);
	render_init_quad(&quad_vao, &quad_vbo, &quad_ebo);
	render_init_line(&line_vao, &line_vbo);
	render_init_color_texture(&color_texture);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	return window;
}

void render_quad(vec2 pos, vec2 size, vec4 color) {
	mat4x4 model;
	mat4x4_identity(model);

	mat4x4_translate(model, pos[0] + size[0] * 0.5, pos[1] + size[1] * 0.5, 0);
	mat4x4_scale_aniso(model, model, size[0], size[1], 1);

	glUniformMatrix4fv(glGetUniformLocation(default_shader, "model"), 1, GL_FALSE, &model[0][0]);
	glUniform4fv(glGetUniformLocation(default_shader, "color"), 1, color);

	glBindVertexArray(quad_vao);
	{
		glBindTexture(GL_TEXTURE_2D, color_texture);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
	}
	glBindVertexArray(0);
}

void render_aabb(void *aabb, vec4 color) {
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
	f32 x = end[0] - start[0];
	f32 y = end[1] - start[1];
	f32 line[6] = {0, 0, 0, x, y, 0};
	mat4x4 model;
	mat4x4_identity(model);

	mat4x4_translate(model, start[0], start[1], 0);

	glUniformMatrix4fv(glGetUniformLocation(default_shader, "model"), 1, GL_FALSE, &model[0][0]);
	glUniform4fv(glGetUniformLocation(default_shader, "color"), 1, color);

	glBindTexture(GL_TEXTURE_2D, color_texture);
	glBindVertexArray(line_vao);
	{
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, line_vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, (sizeof line), &line);
		glDrawArrays(GL_LINES, 0, 2);
		glDisableVertexAttribArray(0);
	}
	glBindVertexArray(0);
}

