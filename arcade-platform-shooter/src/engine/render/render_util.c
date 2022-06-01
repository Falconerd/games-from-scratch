#include <glad/glad.h>
#include <stdlib.h>
#include <stdio.h>
#include "../io.h"
#include "../types.h"

u32 render_shader_create(const char *vert_path, const char *frag_path) {
	int success;
	char log[512];

	char *vertex_source = io_file_read(vert_path);
	if (!vertex_source) {
		exit(1);
	}

	u32 vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, (const char *const *)&vertex_source, NULL);
	glCompileShader(vertex_shader);
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertex_shader, 512, NULL, log);
		printf("Error compiling vertex shader. %s\n", log);
		exit(1);
	}

	char *fragment_source = io_file_read(frag_path);
	if (!fragment_source) {
		exit(1);
	}

	u32 fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, (const char *const *)&fragment_source, NULL);
	glCompileShader(fragment_shader);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragment_shader, 512, NULL, log);
		printf("Error compiling fragment shader. %s\n", log);
		exit(1);
	}

	u32 shader = glCreateProgram();
	glAttachShader(shader, vertex_shader);
	glAttachShader(shader, fragment_shader);
	glLinkProgram(shader);
	glGetProgramiv(shader, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shader, 512, NULL, log);
		printf("Error linking shader. %s\n", log);
		exit(1);
	}

	free(vertex_source);
	free(fragment_source);

	return shader;
}

