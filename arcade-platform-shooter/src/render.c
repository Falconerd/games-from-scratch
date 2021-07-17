#include "shared.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../deps/lib/stb_image.h"

static Render_Context *context;

static void framebuffer_size_callback(GLFWwindow *window, int width, int height);
static void texture_setup(u32 texture);
static u32 shader_setup(const char *vert_path, const char *frag_path);

void render_setup(Render_Context *render_context) {
	context = render_context;
	// Setup the window.
	glfwSetErrorCallback(error_and_exit);
	if (!glfwInit()) {
		error_and_exit(-1, "Failed to init GLFW");
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	context->window = glfwCreateWindow(WIDTH * SCALE, HEIGHT * SCALE, GAME_TITLE, NULL, NULL);
	if (!context->window) {
		error_and_exit(-1, "Failed to create window");
	}

	glfwMakeContextCurrent(context->window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		error_and_exit(-1, "Failed to init GLAD");
	}

	glViewport(0, 0, WIDTH * SCALE, HEIGHT * SCALE);
	glfwSetFramebufferSizeCallback(context->window, framebuffer_size_callback);

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

	// Setup text shader and texture.
	context->text_shader = shader_setup("./shaders/text.vert", "./shaders/text.frag");
	context->text_texture = render_texture_create("./assets/font_gravity_8x10.png");

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

static void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
	glViewport(0, 0, width, height);
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

void render_text(f32 x, f32 y, const char *text, vec4 color) {
	u32 length = strlen(text);
	for (u32 i = 0; i < length; ++i) {
		// for (const char *p = text; *p; ++p) {
		if (text[i] - 'A' >= 0 && text[i] - 'A' <= 25) {
			// printf("char: %u\n", (u8)text[i] - 'A');
		} else if (text[i] - '0' >= 0 && text[i] - '0' <= 9) {
			// printf("num: %u\n", (u8)text[i] - '0');
		}
		m4 model;
		mat4x4_identity(model);

		mat4x4_translate(model, x, y, 0);
		mat4x4_scale_aniso(model, model, 8, 8, 1.0f);

		f32 offset = 0.0f;
		f32 width = 8.0f / 288.0f;

		glUniformMatrix4fv(glGetUniformLocation(context->text_shader, "model"), 1, GL_FALSE, &model[0][0]);
		glUniform4fv(glGetUniformLocation(context->text_shader, "color"), 1, color);
		glUniform1fv(glGetUniformLocation(context->text_shader, "offset"), 1, &offset);
		glUniform1fv(glGetUniformLocation(context->text_shader, "width"), 1, &offset);

		glBindTexture(GL_TEXTURE_2D, context->text_texture);
		glBindVertexArray(context->quad_vao);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}
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
