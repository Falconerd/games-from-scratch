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

#define GRAVITY 4
#define TERMINAL_VELOCITY 15

typedef enum direction {
	LEFT   = 1,
	RIGHT  = 2,
	TOP    = 4,
	BOTTOM = 8
} DIRECTION;

#define MAX_ENEMIES 128
#define MAX_BULLETS 64
#define MAX_ROCKETS 8

typedef struct player {
	vec2 min;
	vec2 max;
	vec2 velocity;
	u32 texture;
} Player;

typedef struct enemy {
	vec2 min;
	vec2 max;
	vec2 velocity;
	u32 texture;
} Enemy;

typedef struct bullet {
	vec2 min;
	vec2 max;
	vec2 velocity;
	u32 texture;
} Bullet;

typedef struct rocket {
	vec2 min;
	vec2 max;
	vec2 velocity;
	u32 texture;
} Rocket;

typedef struct terrain {
	vec2 min;
	vec2 max;
} Terrain;

typedef struct trigger {
	vec2 min;
	vec2 max;
} Trigger;

// Context structs.

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

typedef struct game_context {
	f32 time_now;
	f32 time_last_frame;
	f32 delta_time;

	u32 jump_key_state;
	f32 spawn_timer;
	f32 box_spawn_timer;

	Player player;
	Enemy enemies[MAX_ENEMIES];
	Bullet bullets[MAX_BULLETS];
	Rocket rockets[MAX_ROCKETS];
	Terrain terrain[10];
	// One for the fire. One for the box.
	Trigger triggers[2];
} Game_Context;

void physics_setup(Game_Context *game_context);
void physics_tick(f32 delta_time);
void physics_reset();

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






