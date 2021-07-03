#ifndef src_shared_h_INCLUDED
#define src_shared_h_INCLUDED
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <time.h>


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

#define GRAVITY 15
#define TERMINAL_VELOCITY 15
#define MAX_ENTITIES 256

#define BODY_IS_FIXED 16 // Use direction enum as 1,2,4,8.
#define BODY_IS_TRIGGER 32
#define BODY_IS_KINEMATIC 64

#define ENTITY_IS_IN_USE 1
#define ENTITY_IS_ENEMY 2
#define ENTITY_IS_FLIPPED 4

typedef enum direction {
	LEFT   = 1,
	RIGHT  = 2,
	TOP    = 4,
	BOTTOM = 8
} DIRECTION;

typedef struct body Body;
typedef void (*Collision_Event)(Body* self, Body *other, DIRECTION direction);

typedef struct aabb {
	vec2 min;
	vec2 max;
} AABB;

struct body {
	u8 flags;
	u8 mask;
	vec2 velocity;
	AABB aabb;
	Collision_Event on_collision;
};

typedef struct entity {
	Body body;
	u8 max_health;
	u8 health;
	u8 flags;
	u32 texture;
	vec2 sprite_offset;
	vec2 sprite_size;
} Entity;

// Context structs.

typedef struct entity_context {
	Entity entities[MAX_ENTITIES];
} Entity_Context;

typedef struct input_context {
	u8 player_input;
} Input_Context;

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

void physics_setup(Entity_Context *entity_context);
void physics_tick(f32 delta_time);
void physics_reset();

Entity *entity_create(u32 texture, vec2 position, vec2 body_size, vec2 sprite_offset, vec2 sprite_size, u8 max_health);
Entity_Context *entity_setup();
void entity_reset();
void entity_destroy(Entity *entity);

void input_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
Input_Context *input_setup();

Render_Context *render_setup(const char *title);
u32 shader_setup();
void render_square(f32 x, f32 y, f32 width, f32 height, vec4 color);
void render_sprite(u32 texture, vec3 position, vec2 size, u8 flipped);
u32 render_create_texture(const char *path);

char *read_file_into_buffer(const char *path);

void error_and_exit(i32 code, const char *message);

#endif

