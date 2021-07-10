#ifndef shared_h_INCLUDED
#define shared_h_INCLUDED
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <time.h>
#include <float.h>

#include "../deps/lib/linmath.h"

// Types.

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define f32 float
#define f64 double
#define i32 int32_t
#define m4 mat4x4

// Globals and flags.

#define SCALE 4
#define WIDTH 256
#define HEIGHT 224

void error_and_exit(i32 code, const char *message);
f32 fsign(f32 a);
f32 fclamp(f32 a, f32 min, f32 max);

// Physics.

typedef struct aabb {
	vec2 position;
	vec2 half_sizes;
	vec2 last_position;
} AABB;

typedef struct body {
	AABB aabb;
	vec2 velocity;
	u32 id;
	u8 is_grounded;
} Body;

typedef struct static_body {
	AABB aabb;
	u32 id;
} Static_Body;

typedef struct hit {
	Body *body;
	vec2 position;
	vec2 delta;
	vec2 normal;
	f32 time;
} Hit;

typedef struct sweep {
	Hit hit;
	vec2 position;
	f32 time;
} Sweep;

typedef struct physics_context {
	u32 body_array_count;
	u32 body_array_max;
	Body *body_array;
	u32 static_body_array_count;
	u32 static_body_array_max;
	Static_Body *static_body_array;
} Physics_Context;

Physics_Context *physics_setup(u32 max_bodies, u32 max_static_bodies);
void physics_tick(f32 delta_time);
Body *physics_body_create(vec2 position, vec2 half_sizes);
Static_Body *physics_static_body_create(vec2 position, vec2 half_sizes);

Hit aabb_intersect_aabb(AABB *self, AABB *other);
Hit aabb_intersect_segment(AABB *self, vec2 position, vec2 delta, f32 padding_x, f32 padding_y);
Sweep aabb_sweep_aabb(AABB *self, AABB *other, vec2 delta);

// Entity.

typedef struct entity {
	u32 texture;
	vec2 sprite_offset;
	vec2 sprite_size;
	u32 body_id;
} Entity;

typedef struct entity_context {
	Entity *entity_array;
	u32 entity_array_count;
	u32 entity_array_max;
} Entity_Context;

Entity_Context *entity_setup(u32 max_entities);
Entity *entity_create(u32 texture, vec2 sprite_offset, vec2 sprite_size);

// Render.

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
void render_square(f32 x, f32 y, f32 width, f32 height, vec4 color);
void render_sprite(u32 texture, vec3 position, vec2 size, u8 flipped);
void render_point(vec2 position, vec4 color);
void render_aabb(AABB aabb, vec4 color);
void render_segment(vec2 start, vec2 end, vec4 color);
void render_ray(vec2 start, vec2 direction, f32 length, vec4 color, u8 arrow);
u32 render_texture_create(const char *path);

// Input output.

char *read_file_into_buffer(const char *path);

// User input.

void input_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

#endif
