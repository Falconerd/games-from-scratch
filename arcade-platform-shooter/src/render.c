#include "shared.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../deps/lib/stb_image.h"

#include <ft2build.h>
#include FT_FREETYPE_H

typedef struct character_data {
	f32 advance_x;
	f32 advance_y;
	f32 bitmap_width;
	f32 bitmap_height;
	f32 bitmap_left;
	f32 bitmap_top;
	f32 offset_x;
} Character_Data;

static Character_Data character_data_array[128];
static u32 text_atlas_width = 0;
static u32 text_atlas_height = 0;

static Render_Context *context;

static void texture_setup(u32 texture);
static u32 shader_setup(const char *vert_path, const char *frag_path);

void render_setup(Render_Context *render_context) {
	context = render_context;

	// Setup the window.
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("Could not init SDL\n");
		exit(1);
	}

	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	context->window = SDL_CreateWindow(GAME_TITLE, 0, 0, WIDTH * SCALE, HEIGHT * SCALE, SDL_WINDOW_OPENGL);
	if (!context->window) {
		printf("Failed to init window: %s\n", SDL_GetError());
		exit(1);
	}

	SDL_GL_CreateContext(context->window);
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

	context->shader = shader_setup("./shaders/default.vert", "./shaders/default.frag");

	// Setup color texture.
	glGenTextures(1, &context->color_texture);
	texture_setup(context->color_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, (u8[4]){255, 255, 255, 255});

	stbi_set_flip_vertically_on_load(1);

	// Setup quad rendering.
	//      Position         Tex coords
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
	glGenVertexArrays(1, &context->quad_vao);
	glGenBuffers(1, &context->quad_vbo);
	glGenBuffers(1, &context->quad_ebo);

	glBindVertexArray(context->quad_vao);
	glBindBuffer(GL_ARRAY_BUFFER, context->quad_vao);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, context->quad_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_indices), quad_indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), NULL);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), (void*)(3 * sizeof(f32)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Setup line rendering.
	f32 line_vertices[] = { 0, 0, 0, 1, 1, 1 };
	glGenVertexArrays(1, &context->line_vao);
	glGenBuffers(1, &context->line_vbo);

	glBindVertexArray(context->line_vao);
	glBindBuffer(GL_ARRAY_BUFFER, context->line_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(line_vertices), line_vertices, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), NULL);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Setup circle shader.
	context->circle_shader = shader_setup("./shaders/circle.vert", "./shaders/circle.frag");

	// Setup text shader.
	context->text_shader = shader_setup("./shaders/text.vert", "./shaders/text.frag");

	glGenVertexArrays(1, &context->text_vao);
	glGenBuffers(1, &context->text_vbo);

	glBindVertexArray(context->text_vao);
	glBindBuffer(GL_ARRAY_BUFFER, context->text_vbo);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), NULL);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Init freetype library.
	FT_Library ft;
	if (FT_Init_FreeType(&ft)) {
		error_and_exit(EXIT_FAILURE, "Could not init freetype.");
	}

	// Load font face.
	FT_Face face;
	if (FT_New_Face(ft, "./assets/PxPlus_IBM_VGA_8x16.ttf", 0, &face)) {
		error_and_exit(EXIT_FAILURE, "Could not load font.");
	}

	FT_Set_Pixel_Sizes(face, 0, 8);

	// Create texture atlas for font.
	for (u32 i = 32; i < 128; ++i) {
		if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
			printf("Failed to load char '%c'\n", i);
			continue;
		}

		text_atlas_width += face->glyph->bitmap.width;
		if (text_atlas_height < face->glyph->bitmap.rows) {
			text_atlas_height = face->glyph->bitmap.rows;
		}
	}

	printf("text_atlas_width: %u, text_atlas_height: %u\n", text_atlas_width, text_atlas_height);

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &context->text_texture);
	glBindTexture(GL_TEXTURE_2D, context->text_texture);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, text_atlas_width, text_atlas_height, 0, GL_RED, GL_UNSIGNED_BYTE, 0);

	for (u32 i = 32, w = 0; i < 128; ++i, ++w) {
		if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
			printf("Failed to load char '%c'\n", i);
			continue;
		}

		character_data_array[w].advance_x = face->glyph->advance.x >> 6;
		character_data_array[w].advance_y = face->glyph->advance.y >> 6;
		character_data_array[w].bitmap_width = face->glyph->bitmap.width;
		character_data_array[w].bitmap_height = face->glyph->bitmap.rows;
		character_data_array[w].bitmap_left = face->glyph->bitmap_left;
		character_data_array[w].bitmap_top = face->glyph->bitmap_top;
		character_data_array[w].offset_x = (f32)w / text_atlas_width;

		glTexSubImage2D(GL_TEXTURE_2D, 0, w, 0, face->glyph->bitmap.width, face->glyph->bitmap.rows,
		                GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
	}

	// Setup projection matrix for each shader.
	mat4x4_ortho(context->projection, 0, WIDTH, 0, HEIGHT, -2.0f, 2.0f);

	glUseProgram(context->shader);
	glUniformMatrix4fv(glGetUniformLocation(context->shader, "projection"), 1, GL_FALSE, &context->projection[0][0]);
	glUseProgram(context->text_shader);
	glUniformMatrix4fv(glGetUniformLocation(context->text_shader, "projection"), 1, GL_FALSE, &context->projection[0][0]);
	glUseProgram(context->circle_shader);
	glUniformMatrix4fv(glGetUniformLocation(context->circle_shader, "projection"), 1, GL_FALSE, &context->projection[0][0]);
}

void render_screen_shake_add(f32 duration, f32 magnitude) {
	context->screen_shake_timer += duration;
	context->screen_shake_magnitude += magnitude;
}

void render_screen_shake(f32 delta_time) {
	if (context->screen_shake_timer <= 0) {
		context->screen_shake_magnitude = 0;
		mat4x4_ortho(context->projection, 0, WIDTH, 0, HEIGHT, -2.0f, 2.0f);
		glUniformMatrix4fv(glGetUniformLocation(context->shader, "projection"), 1, GL_FALSE, &context->projection[0][0]);
		return;
	} else {
		context->screen_shake_timer -= delta_time;
	}
	f32 x = frandr(-context->screen_shake_magnitude, context->screen_shake_magnitude);
	f32 y = frandr(-context->screen_shake_magnitude, context->screen_shake_magnitude);
	glUseProgram(context->shader);
	mat4x4_ortho(context->projection, 0 + x, WIDTH + x, 0 + y, HEIGHT + y, -2.0f, 2.0f);
	glUniformMatrix4fv(glGetUniformLocation(context->shader, "projection"), 1, GL_FALSE, &context->projection[0][0]);
}

static void texture_setup(u32 texture) {
	glBindTexture(GL_TEXTURE_2D, texture);
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

void render_text(const char *text, f32 x, f32 y, vec4 color) {
	Point coords[6 * strlen(text)];

	u32 n = 0;

	for (const char *p = text; *p; ++p) {
		Character_Data cd = character_data_array[*p];
		f32 x2 = x + cd.bitmap_left;
		f32 y2 = -y - cd.bitmap_top;
		f32 w = cd.bitmap_width;
		f32 h = cd.bitmap_height;

		x += cd.advance_x;
		y += cd.advance_y;

		// Skip glyphs that have no pixels.
		if (!w || !h)
			continue;

		coords[n++] = (Point){x2, -y2, cd.offset_x, 0};
		coords[n++] = (Point){x2 + w, -y2, cd.offset_x + cd.bitmap_width / text_atlas_width, 0};
		coords[n++] = (Point){x2, -y2 - h, cd.offset_x, cd.bitmap_height / text_atlas_height};
		coords[n++] = (Point){x2 + w, -y2, cd.offset_x + cd.bitmap_width / text_atlas_width, 0};
		coords[n++] = (Point){x2, -y2 - h, cd.offset_x, cd.bitmap_height / text_atlas_height};
		coords[n++] = (Point){x2 + w, -y2 - h, cd.offset_x + cd.bitmap_width / text_atlas_width};
		glUseProgram(context->shader);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		render_quad(x2, -y2, w, h, (vec4){0, 1, 0, 1});
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	glUseProgram(context->text_shader);

	glUniform4fv(glGetUniformLocation(context->text_shader, "color"), 1, color);

	glBindTexture(GL_TEXTURE_2D, context->text_texture);
	glBindVertexArray(context->text_vao);
	glBindBuffer(GL_ARRAY_BUFFER, context->text_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof coords, coords, GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLES, 0, n);
}

void render_circle(f32 x, f32 y, f32 radius, vec4 color) {
	m4 model;
	mat4x4_identity(model);

	mat4x4_translate(model, WIDTH / 2, HEIGHT / 2, 0.0f);
	mat4x4_scale_aniso(model, model, WIDTH, HEIGHT, 1.0f);

	float r[] = {radius * SCALE};

	glUniformMatrix4fv(glGetUniformLocation(context->circle_shader, "model"), 1, GL_FALSE, &model[0][0]);
	glUniform4fv(glGetUniformLocation(context->circle_shader, "color"), 1, color);
	glUniform2fv(glGetUniformLocation(context->circle_shader, "position"), 1, (vec2){x * SCALE, y * SCALE});
	glUniform1fv(glGetUniformLocation(context->circle_shader, "radius"), 1, &r[0]);

	glBindTexture(GL_TEXTURE_2D, context->color_texture);
	glBindVertexArray(context->quad_vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void render_quad(f32 x, f32 y, f32 width, f32 height, vec4 color) {
	m4 model;
	mat4x4_identity(model);

	mat4x4_translate(model, x + width * 0.5f, y + height * 0.5f, 0.0f);
	mat4x4_scale_aniso(model, model, width, height, 1.0f);

	glUniformMatrix4fv(glGetUniformLocation(context->shader, "model"), 1, GL_FALSE, &model[0][0]);
	glUniform4fv(glGetUniformLocation(context->shader, "color"), 1, color);

	glBindTexture(GL_TEXTURE_2D, context->color_texture);
	glBindVertexArray(context->quad_vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void render_point(vec2 position, vec4 color) {
	m4 model;
	mat4x4_identity(model);

	mat4x4_translate(model, position[0], position[1], 0);

	glUniformMatrix4fv(glGetUniformLocation(context->shader, "model"), 1, GL_FALSE, &model[0][0]);
	glUniform4fv(glGetUniformLocation(context->shader, "color"), 1, color);

	glBindTexture(GL_TEXTURE_2D, context->color_texture);
	glBindVertexArray(context->quad_vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void render_aabb(AABB aabb, vec4 color) {
	render_quad(aabb.position[0] - aabb.half_sizes[0], aabb.position[1] - aabb.half_sizes[1], aabb.half_sizes[0] * 2, aabb.half_sizes[1] * 2, color);
}

void render_segment(vec2 start, vec2 end, vec4 color) {
	f32 x = end[0] - start[0];
	f32 y = end[1] - start[1];
	f32 line[6] = {0, 0, 0, x, y, 0};
	m4 model;
	mat4x4_identity(model);

	mat4x4_translate(model, start[0], start[1], 0);

	glUniformMatrix4fv(glGetUniformLocation(context->shader, "model"), 1, GL_FALSE, &model[0][0]);
	glUniform4fv(glGetUniformLocation(context->shader, "color"), 1, color);

	glBindTexture(GL_TEXTURE_2D, context->color_texture);
	glBindVertexArray(context->line_vao);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, context->line_vbo);
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

void render_sprite(u32 texture, vec3 position, vec2 size, f32 rotation, vec4 color, u8 is_flipped) {
	m4 model;
	mat4x4_identity(model);

	mat4x4_translate(model, position[0] + size[0] * 0.5f, position[1] + size[1] * 0.5f, 0.0f);
	mat4x4_rotate(model, model, 0, 0, 1, rotation);
	mat4x4_scale_aniso(model, model, is_flipped ? -size[0] : size[0], size[1], 1.0f);

	glUniformMatrix4fv(glGetUniformLocation(context->shader, "model"), 1, GL_FALSE, &model[0][0]);
	glUniform4fv(glGetUniformLocation(context->shader, "color"), 1, color);

	glBindTexture(GL_TEXTURE_2D, texture);
	glBindVertexArray(context->quad_vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

u32 render_texture_create(const char *path) {
	u32 texture;
	glGenTextures(1, &texture);
	texture_setup(texture);
	i32 width, height, channel_count;
	u8 *image_data = stbi_load(path, &width, &height, &channel_count, 0);
	if (!image_data) {
		error_and_exit(EXIT_FAILURE, "Failed to load image\n");
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
	stbi_image_free(image_data);
	return texture;
}
