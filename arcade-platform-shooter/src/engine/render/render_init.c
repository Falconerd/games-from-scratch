#include <glad/glad.h>
#include "../types.h"
#include "render_internal.h"

SDL_Window *render_init_window(f32 width, f32 height) {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("Could not init SDL\n");
		exit(1);
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	SDL_Window *window = SDL_CreateWindow("Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL);
	if (!window) {
		printf("Failed to init window: %s\n", SDL_GetError());
		exit(1);
	}

	return window;
}

void render_init_context(SDL_Window *window) {
	SDL_GL_CreateContext(window);
	if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
		printf("Failed to init GLAD\n");
		exit(1);
	}

	printf("OpenGL Loaded\n");
	printf("Vendor:   %s\n", glGetString(GL_VENDOR));
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
	printf("Version:  %s\n", glGetString(GL_VERSION));
}

void render_init_shaders(u32 *default_shader, mat4x4 projection, f32 width, f32 height) {
	*default_shader = render_shader_create("./shaders/default.vert", "./shaders/default.frag");

	mat4x4_ortho(projection, 0, width, 0, height, -2, 2);

	glUseProgram(*default_shader);
	glUniformMatrix4fv(glGetUniformLocation(*default_shader, "projection"), 1, GL_FALSE, &projection[0][0]);
}

void render_init_color_texture(u32 *color_texture) {
	glGenTextures(1, color_texture);
	glBindTexture(GL_TEXTURE_2D, *color_texture);
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, (u8[4]){255, 255, 255, 255});
	}
	glBindTexture(GL_TEXTURE_2D, 0);
}

void render_init_quad(u32 *quad_vao, u32 *quad_vbo, u32 *quad_ebo) {
	f32 vertices[] = {
		 0.5,  0.5, 0, 1, 1,
		 0.5, -0.5, 0, 1, 0,
		-0.5, -0.5, 0, 0, 0,
		-0.5,  0.5, 0, 0, 1
	};
	u32 indices[] = {
		0, 1, 3,
		1, 2, 3
	};

	glGenVertexArrays(1, quad_vao);
	glGenBuffers(1, quad_vbo);
	glGenBuffers(1, quad_ebo);

	glBindVertexArray(*quad_vao);
	{
		glBindBuffer(GL_ARRAY_BUFFER, *quad_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *quad_ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), NULL);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), (void*)(3 * sizeof(f32)));
		glEnableVertexAttribArray(1);
	}
	glBindVertexArray(0);
}

void render_init_line(u32 *line_vao, u32 *line_vbo) {
	f32 vertices[6] = {0, 0, 0, 1, 1, 1};

	glGenVertexArrays(1, line_vao);
	glGenBuffers(1, line_vbo);

	glBindVertexArray(*line_vao);
	{
		glBindBuffer(GL_ARRAY_BUFFER, *line_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), NULL);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	glBindVertexArray(0);
}

void render_init_font(Font *font, FT_Library ft) {
	if (FT_New_Face(ft, font->path, 0, &font->face)) {
		printf("Could not load font: %s. Exiting.\n", font->path);
		exit(1);
	}

	FT_Set_Pixel_Sizes(font->face, 0, font->size);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	for (u32 i = 0, n = 0; i < 128; ++i) {
		if (FT_Load_Char(font->face, i, FT_LOAD_RENDER)) {
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
			font->face->glyph->bitmap.width,
			font->face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			font->face->glyph->bitmap.buffer
		);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		Character_Data *character_data = &font->character_data_array[n++];
		character_data->texture = texture;
		character_data->advance_x = font->face->glyph->advance.x;
		character_data->advance_y = font->face->glyph->advance.y;
		character_data->width = font->face->glyph->bitmap.width;
		character_data->height = font->face->glyph->bitmap.rows;
		character_data->top = font->face->glyph->bitmap_top;
		character_data->left = font->face->glyph->bitmap_left;
	}

	FT_Done_Face(font->face);
}

FT_Library render_init_text_begin(u32 *shader, u32 *vao, u32 *vbo, f32 width, f32 height, f32 scale) {
	*shader = render_shader_create("./shaders/text.vert", "./shaders/text.frag");
	mat4x4 text_projection;
	mat4x4_identity(text_projection);
	mat4x4_ortho(text_projection, 0, width * scale, 0, height * scale, -2, 2);
	glUseProgram(*shader);
	glUniformMatrix4fv(glGetUniformLocation(*shader, "projection"), 1, GL_FALSE, &text_projection[0][0]);

	// Init freetype library.
	FT_Library ft;
	if (FT_Init_FreeType(&ft)) {
		printf("Could not initialize freetype. Exiting.\n");
		exit(1);
	}

	glGenVertexArrays(1, vao);
	glGenBuffers(1, vbo);
	glBindVertexArray(*vao);
	glBindBuffer(GL_ARRAY_BUFFER, *vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(f32) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return ft;
}

void render_init_text_end(FT_Library ft) {
	FT_Done_FreeType(ft);
}

