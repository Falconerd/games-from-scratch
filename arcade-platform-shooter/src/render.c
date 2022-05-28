#include "shared.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../deps/lib/stb_image.h"

#include <ft2build.h>
#include FT_FREETYPE_H

Render_State render_state = {0};
static Render_State *state = &render_state;

typedef struct character_data {
	u32 texture;
	u32 advance_x;
	u32 advance_y;
	i32 width;
	i32 height;
	i32 left;
	i32 top;
} Character_Data;

static Character_Data character_data_array[128];

static FT_Face face;
static FT_GlyphSlot g;

static void texture_setup(u32 texture_id);
static u32 shader_setup(const char *vert_path, const char *frag_path);

void render_setup() {

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
	// SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	
	// Setup the window.
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("Could not init SDL\n");
		exit(1);
	}

	state->window = SDL_CreateWindow(GAME_TITLE, 0, 0, WIDTH * SCALE, HEIGHT * SCALE, SDL_WINDOW_OPENGL);
	if (!state->window) {
		printf("Failed to init window: %s\n", SDL_GetError());
		exit(1);
	}

	SDL_GL_CreateContext(state->window);
	if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
		error_and_exit(-1, "Failed to init GLAD");
	}

	printf("OpenGL loaded\n");
	printf("Vendor:   %s\n", glGetString(GL_VENDOR));
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
	printf("Version:  %s\n", glGetString(GL_VERSION));

	glViewport(0, 0, WIDTH * SCALE, HEIGHT * SCALE);

	// Setup shader and buffers.
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	state->shader = shader_setup("./shaders/default.vert", "./shaders/default.frag");

	// Setup color texture.
	glGenTextures(1, &state->color_texture);
	texture_setup(state->color_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, (u8[4]){255, 255, 255, 255});

	stbi_set_flip_vertically_on_load(1);

	// Setup quad rendering.
	f32 quad_vertices[] = {
		 0.5f,  0.5f, 0, 1.0f, 1.0f,
		 0.5f, -0.5f, 0, 1.0f, 0.0f,
		-0.5f, -0.5f, 0, 0.0f, 0.0f,
		-0.5f,  0.5f, 0, 0.0f, 1.0f
	};
	u32 quad_indices[] = {
		0, 1, 3,
		1, 2, 3
	};
	glGenVertexArrays(1, &state->quad_vao);
	glGenBuffers(1, &state->quad_vbo);
	glGenBuffers(1, &state->quad_ebo);

	glBindVertexArray(state->quad_vao);
	glBindBuffer(GL_ARRAY_BUFFER, state->quad_vao);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state->quad_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_indices), quad_indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), NULL);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), (void*)(3 * sizeof(f32)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Setup line rendering.
	f32 line_vertices[] = { 0, 0, 0, 1, 1, 1 };
	glGenVertexArrays(1, &state->line_vao);
	glGenBuffers(1, &state->line_vbo);

	glBindVertexArray(state->line_vao);
	glBindBuffer(GL_ARRAY_BUFFER, state->line_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(line_vertices), line_vertices, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), NULL);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Setup circle shader.
	state->circle_shader = shader_setup("./shaders/circle.vert", "./shaders/circle.frag");

	// Setup text shader.
	state->text_shader = shader_setup("./shaders/text.vert", "./shaders/text.frag");

	glGenVertexArrays(1, &state->text_vao);
	glGenBuffers(1, &state->text_vbo);

	glBindVertexArray(state->text_vao);
	glBindBuffer(GL_ARRAY_BUFFER, state->text_vbo);
	glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), NULL);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Init freetype library.
	FT_Library ft;
	if (FT_Init_FreeType(&ft)) {
		error_and_exit(EXIT_FAILURE, "Could not init freetype.");
	}

	// Load font face.
	if (FT_New_Face(ft, "./assets/8-BIT_WONDER.TTF", 0, &face)) {
		error_and_exit(EXIT_FAILURE, "Could not load font.");
	}

	FT_Set_Pixel_Sizes(face, 0, 12 * SCALE);

	g = face->glyph;

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	for (u32 i = 0, n = 0; i < 128; ++i) {
		if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
			printf("Failed to load glyph '%c'\n", i);
			continue;
		}

		u32 texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			g->bitmap.width,
			g->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			g->bitmap.buffer
		);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		Character_Data *character_data = &character_data_array[n++];
		character_data->texture = texture;
		character_data->advance_x = g->advance.x;
		character_data->advance_y = g->advance.y;
		character_data->width = g->bitmap.width;
		character_data->height = g->bitmap.rows;
		character_data->top = g->bitmap_top;
		character_data->left = g->bitmap_left;
	}

	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	glGenVertexArrays(1, &state->text_vao);
	glGenBuffers(1, &state->text_vbo);
	glBindVertexArray(state->text_vao);
	glBindBuffer(GL_ARRAY_BUFFER, state->text_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(f32) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Setup projection matrix for each shader.
	mat4x4_ortho(state->projection, 0, WIDTH, 0, HEIGHT, -2.0f, 2.0f);

	glUseProgram(state->shader);
	glUniformMatrix4fv(glGetUniformLocation(state->shader, "projection"), 1, GL_FALSE, &state->projection[0][0]);
	glUseProgram(state->circle_shader);
	glUniformMatrix4fv(glGetUniformLocation(state->circle_shader, "projection"), 1, GL_FALSE, &state->projection[0][0]);

	mat4x4 text_projection;
	mat4x4_identity(text_projection);
	mat4x4_ortho(text_projection, 0, WIDTH * SCALE, 0, HEIGHT * SCALE, -2.0f, 2.0f);
	glUseProgram(state->text_shader);
	glUniformMatrix4fv(glGetUniformLocation(state->text_shader, "projection"), 1, GL_FALSE, &text_projection[0][0]);
}

void render_screen_shake_add(f32 duration, f32 magnitude) {
	state->screen_shake_timer += duration;
	state->screen_shake_magnitude += magnitude;
}

void render_screen_shake(f32 delta_time) {
	if (state->screen_shake_timer <= 0) {
		state->screen_shake_magnitude = 0;
		mat4x4_ortho(state->projection, 0, WIDTH, 0, HEIGHT, -2.0f, 2.0f);
		glUniformMatrix4fv(glGetUniformLocation(state->shader, "projection"), 1, GL_FALSE, &state->projection[0][0]);
		return;
	} else {
		state->screen_shake_timer -= delta_time;
	}
	f32 x = frandr(-state->screen_shake_magnitude, state->screen_shake_magnitude);
	f32 y = frandr(-state->screen_shake_magnitude, state->screen_shake_magnitude);
	glUseProgram(state->shader);
	mat4x4_ortho(state->projection, 0 + x, WIDTH + x, 0 + y, HEIGHT + y, -2.0f, 2.0f);
	glUniformMatrix4fv(glGetUniformLocation(state->shader, "projection"), 1, GL_FALSE, &state->projection[0][0]);
}

static void texture_setup(u32 texture_id) {
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);   
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

static u32 shader_setup(const char *vert_path, const char *frag_path) {
	int success;
	char log[512];
	char *vertex_source = read_file_into_buffer(vert_path);
	uint32_t vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, (const char *const *)&vertex_source, NULL);
	glCompileShader(vertex_shader);
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertex_shader, 512, NULL, log);
		error_and_exit(-1, log);
	}

	char *fragment_source = read_file_into_buffer(frag_path);
	uint32_t fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, (const char *const *)&fragment_source, NULL);
	glCompileShader(fragment_shader);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragment_shader, 512, NULL, log);
		error_and_exit(-1, log);
	}

	u32 shader = glCreateProgram();
	glAttachShader(shader, vertex_shader);
	glAttachShader(shader, fragment_shader);
	glLinkProgram(shader);
	glGetProgramiv(shader, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shader, 512, NULL, log);
		error_and_exit(-1, log);
	}

	free(vertex_source);
	free(fragment_source);

	return shader;
}

typedef struct point {
	f32 x, y, s, t;
} Point;

void render_text(const char *text, f32 x, f32 y, vec4 color, u8 is_centered) {
	glUseProgram(state->text_shader);
	glUniform4fv(glGetUniformLocation(state->text_shader, "color"), 1, color);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(state->text_vao);

	x *= SCALE;
	y *= SCALE;

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
		glBindBuffer(GL_ARRAY_BUFFER, state->text_vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof vertices, vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		x += (cd.advance_x / 64);
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void render_circle(f32 x, f32 y, f32 radius, vec4 color) {
	mat4x4 model;
	mat4x4_identity(model);

	mat4x4_translate(model, WIDTH / 2, HEIGHT / 2, 0.0f);
	mat4x4_scale_aniso(model, model, WIDTH, HEIGHT, 1.0f);

	float r[] = {radius * SCALE};

	glUniformMatrix4fv(glGetUniformLocation(state->circle_shader, "model"), 1, GL_FALSE, &model[0][0]);
	glUniform4fv(glGetUniformLocation(state->circle_shader, "color"), 1, color);
	glUniform2fv(glGetUniformLocation(state->circle_shader, "position"), 1, (vec2){x * SCALE, y * SCALE});
	glUniform1fv(glGetUniformLocation(state->circle_shader, "radius"), 1, &r[0]);

	glBindTexture(GL_TEXTURE_2D, state->color_texture);
	glBindVertexArray(state->quad_vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void render_quad(f32 x, f32 y, f32 width, f32 height, vec4 color) {
	mat4x4 model;
	mat4x4_identity(model);

	mat4x4_translate(model, x + width * 0.5f, y + height * 0.5f, 0.0f);
	mat4x4_scale_aniso(model, model, width, height, 1.0f);

	glUniformMatrix4fv(glGetUniformLocation(state->shader, "model"), 1, GL_FALSE, &model[0][0]);
	glUniform4fv(glGetUniformLocation(state->shader, "color"), 1, color);

	glBindTexture(GL_TEXTURE_2D, state->color_texture);
	glBindVertexArray(state->quad_vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void render_point(vec2 position, vec4 color) {
	mat4x4 model;
	mat4x4_identity(model);

	mat4x4_translate(model, position[0], position[1], 0);

	glUniformMatrix4fv(glGetUniformLocation(state->shader, "model"), 1, GL_FALSE, &model[0][0]);
	glUniform4fv(glGetUniformLocation(state->shader, "color"), 1, color);

	glBindTexture(GL_TEXTURE_2D, state->color_texture);
	glBindVertexArray(state->quad_vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void render_aabb(AABB aabb, vec4 color) {
	render_quad(aabb.position[0] - aabb.half_sizes[0], aabb.position[1] - aabb.half_sizes[1], aabb.half_sizes[0] * 2, aabb.half_sizes[1] * 2, color);
}

void render_segment(vec2 start, vec2 end, vec4 color) {
	f32 x = end[0] - start[0];
	f32 y = end[1] - start[1];
	f32 line[6] = {0, 0, 0, x, y, 0};
	mat4x4 model;
	mat4x4_identity(model);

	mat4x4_translate(model, start[0], start[1], 0);

	glUniformMatrix4fv(glGetUniformLocation(state->shader, "model"), 1, GL_FALSE, &model[0][0]);
	glUniform4fv(glGetUniformLocation(state->shader, "color"), 1, color);

	glBindTexture(GL_TEXTURE_2D, state->color_texture);
	glBindVertexArray(state->line_vao);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, state->line_vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, (sizeof line), &line);
	glDrawArrays(GL_LINES, 0, 2);
	glDisableVertexAttribArray(0);
}

void render_ray(vec2 start, vec2 direction, f32 length, vec4 color, u8 arrow) {
	vec2 normal;
	vec2_norm(normal, direction);
	vec2 end = {start[0] + normal[0] * length, start[1] + normal[1] * length};
	render_segment(start, end, color);
	if (arrow) {
		{
			vec2 position = {end[0] - direction[0] * 2 + direction[1] * 2,
			                 end[1] - direction[1] * 2 - direction[0] * 2};
			render_segment(end, position, color);
		}
		{
			vec2 position = {end[0] - direction[0] * 2 - direction[1] * 2,
			                 end[1] - direction[1] * 2 + direction[0] * 2};
			render_segment(end, position, color);
		}
	}
}

void render_sprite(Texture texture, f32 size[2], vec3 position, f32 tex_coords[8], f32 rotation, vec4 color, u8 is_flipped) {
	f32 vertices[4][5] = {
		{ 0.5f,  0.5f, 0, 1.0f, 1.0f},
		{ 0.5f, -0.5f, 0, 1.0f, 0.0f},
		{-0.5f, -0.5f, 0, 0.0f, 0.0f},
		{-0.5f,  0.5f, 0, 0.0f, 1.0f}
	};

	if (tex_coords) {
		vertices[0][3] = tex_coords[0];
		vertices[0][4] = tex_coords[1];
		vertices[1][3] = tex_coords[2];
		vertices[1][4] = tex_coords[3];
		vertices[2][3] = tex_coords[4];
		vertices[2][4] = tex_coords[5];
		vertices[3][3] = tex_coords[6];
		vertices[3][4] = tex_coords[7];
	}

	f32 width = texture.width;
	f32 height = texture.height;

	if (size != NULL) {
		width = size[0];
		height = size[1];
	}

	glBindBuffer(GL_ARRAY_BUFFER, state->quad_vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof vertices, vertices);

	mat4x4 model;
	mat4x4_identity(model);

	mat4x4_translate(model, position[0] + width * 0.5f, position[1] + height * 0.5f, 0.0f);
	mat4x4_rotate(model, model, 0, 0, 1, rotation);
	mat4x4_scale_aniso(model, model, is_flipped ? -width : width, height, 1.0f);

	glUniformMatrix4fv(glGetUniformLocation(state->shader, "model"), 1, GL_FALSE, &model[0][0]);
	glUniform4fv(glGetUniformLocation(state->shader, "color"), 1, color);

	glBindTexture(GL_TEXTURE_2D, texture.id);
	glBindVertexArray(state->quad_vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void render_sprite_sheet_frame(Sprite_Sheet sprite_sheet, u8 row, u8 column, vec3 position, f32 rotation, vec4 color, u8 is_flipped) {
	f32 w = 1.0f / ((f32)sprite_sheet.texture.width / (f32)sprite_sheet.frame_width);
	f32 h = 1.0f / ((f32)sprite_sheet.texture.height / (f32)sprite_sheet.frame_height);
	f32 x = (f32)column * w;
	f32 y = (f32)row * h;
	f32 tex_coords[8] = {x + w, y + h, x + w, y, x, y, x, y + h};
	f32 size_override[2] = {(f32)sprite_sheet.frame_width, (f32)sprite_sheet.frame_height};

	render_sprite(sprite_sheet.texture, size_override, position, tex_coords, rotation, color, is_flipped);
}

Texture render_texture_create(const char *path) {
	Texture texture = {0};
	glGenTextures(1, &texture.id);
	texture_setup(texture.id);
	u8 *image_data = stbi_load(path, &texture.width, &texture.height, &texture.channel_count, 0);
	if (!image_data) {
		error_and_exit(EXIT_FAILURE, "Failed to load image\n");
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.width, texture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
	stbi_image_free(image_data);
	return texture;
}
