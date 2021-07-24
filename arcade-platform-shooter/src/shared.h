#ifndef shared_h_INCLUDED
#define shared_h_INCLUDED
#include <glad/glad.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <time.h>
#include <float.h>

#include "../deps/lib/linmath.h"

////////////////////////////////////////////////////////////////////////
// Defined types.
////////////////////////////////////////////////////////////////////////

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define f32 float
#define f64 double
#define i8 int8_t
#define i32 int32_t
#define m4 mat4x4
#define v2 vec2
#define v3 vec3
#define v4 vec4

////////////////////////////////////////////////////////////////////////
// Defines and flags.
////////////////////////////////////////////////////////////////////////

#define DEBUG 1

#define PI 3.14159265f

#define GAME_TITLE "Mega Box Crate"

#define SCALE 4
#define WIDTH 256
#define HEIGHT 224

#define GRAVITY -20
#define TERMINAL_VELOCITY -200

#define MAX_ENTITIES 256
#define MAX_STATIC_BODIES 20
#define MAX_TRIGGERS 10

////////////////////////////////////////////////////////////////////////
// Shared functions.
////////////////////////////////////////////////////////////////////////

void error_and_exit(i32 code, const char *message);
f32 fsign(f32 a);
f32 fclamp(f32 a, f32 min, f32 max);
f32 frandr(f32 min, f32 max);

f32 vec2_dist(vec2 a, vec2 b);
f32 vec2_sqr_dist(vec2 a, vec2 b);

////////////////////////////////////////////////////////////////////////
// Typedefs.
////////////////////////////////////////////////////////////////////////

typedef struct hit Hit;
typedef struct aabb AABB;
typedef struct static_body Static_Body;
typedef struct trigger Trigger;
typedef struct physics_context Physics_Context;

typedef struct entity Entity;
typedef struct entity_context Entity_Context;

typedef struct render_context Render_Context;

typedef void (*On_Collide_Function)(u32 self_id, u32 other_id, Hit hit);
typedef void (*On_Collide_Static_Function)(u32 self_id, u32 static_body_id, Hit hit);
typedef void (*On_Trigger_Function)(u32 self_id, u32 trigger_id, Hit hit);

////////////////////////////////////////////////////////////////////////
// Physics.
////////////////////////////////////////////////////////////////////////

struct hit {
	vec2 position;
	vec2 delta;
	vec2 normal;
	f32 time;
};

struct aabb {
	vec2 position;
	vec2 half_sizes;
};

struct static_body {
	AABB aabb;
	u8 layer_mask;
};

struct trigger {
	AABB aabb;
	u32 id;
	On_Trigger_Function on_trigger;
};

struct physics_context {
	u32 static_body_array_count;
	u32 static_body_array_max;
	Static_Body *static_body_array;
	u32 trigger_array_count;
	u32 trigger_array_max;
	Trigger *trigger_array;
	u8 mask_array[5];
};

void physics_setup(Physics_Context *physics_context);
void physics_tick(f32 delta_time, Entity *entity_array);
Static_Body *physics_static_body_create(f32 x, f32 y, f32 half_width, f32 half_height, u8 layer_mask);
Trigger *physics_trigger_create(f32 x, f32 y, f32 half_width, f32 half_height);
Hit *aabb_intersect_aabb(AABB self, AABB other);
void physics_cleanup();

////////////////////////////////////////////////////////////////////////
// Entity.
////////////////////////////////////////////////////////////////////////

struct entity {
	AABB aabb;
	vec2 velocity;
	vec2 desired_velocity;
	vec2 acceleration;
	u32 texture;
	vec2 sprite_size;
	vec2 sprite_offset;

	vec4 sprite_color;
	vec4 desired_sprite_color;
	vec4 sprite_color_delta;

	f32 rotation;

	On_Collide_Function on_collide;
	On_Collide_Static_Function on_collide_static;
	f32 time_to_live;
	u8 is_in_use;
	u8 is_flipped;
	u8 is_grounded;
	u8 is_kinematic;
	u8 layer_mask;
	i8 health;
};

struct entity_context {
	Entity *entity_array;
};

void entity_setup(Entity_Context *entity_context);
u32 entity_create(u32 texture, f32 x, f32 y, f32 collider_half_width, f32 collider_half_height, f32 sprite_width, f32 sprite_height, f32 sprite_offset_x, f32 sprite_offset_y, u32 layer_mask);
void entity_destroy(u32 index);

////////////////////////////////////////////////////////////////////////
// Render.
////////////////////////////////////////////////////////////////////////

struct render_context {
	SDL_Window *window;
	SDL_Renderer *renderer;
	m4 projection;
	u32 color_texture;
	u32 shader;
	u32 quad_vao;
	u32 quad_vbo;
	u32 quad_ebo;
	u32 line_vao;
	u32 line_vbo;
	u32 text_vao;
	u32 text_vbo;
	u32 text_shader;
	u32 text_texture;
	u32 circle_shader;

	f32 screen_shake_timer;
	f32 screen_shake_magnitude;
};

void render_setup(Render_Context *render_context);
void render_quad(f32 x, f32 y, f32 width, f32 height, vec4 color);
void render_circle(f32 x, f32 y, f32 radius, vec4 color);
void render_text(const char *text, f32 x, f32 y, vec4 color, u8 is_centered);
void render_sprite(u32 texture, vec3 position, vec2 size, f32 rotation, vec4 color, u8 is_flipped);
void render_point(vec2 position, vec4 color);
void render_aabb(AABB aabb, vec4 color);
void render_segment(vec2 start, vec2 end, vec4 color);
void render_ray(vec2 start, vec2 direction, f32 length, vec4 color, u8 arrow);
u32 render_texture_create(const char *path);
void render_screen_shake_add(f32 duration, f32 magnitude);
void render_screen_shake(f32 delta_time);

////////////////////////////////////////////////////////////////////////
// Input output.
////////////////////////////////////////////////////////////////////////

char *read_file_into_buffer(const char *path);
int write_buffer_into_file(void *buffer, size_t length, const char *path);

////////////////////////////////////////////////////////////////////////
// User input.
////////////////////////////////////////////////////////////////////////

typedef struct input_context {
	u8 left;
	u8 jump;
	u8 right;
	u8 shoot;
} Input_Context;

void input_setup(Input_Context *input_context);
// void input_key_callback(SDL_window *window, int key, int scancode, int action, int mods);

#endif
