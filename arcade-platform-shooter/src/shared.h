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

////////////////////////////////////////////////////////////////////////
// Defined types.
////////////////////////////////////////////////////////////////////////

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define f32 float
#define f64 double
#define i32 int32_t
#define m4 mat4x4

////////////////////////////////////////////////////////////////////////
// Globals and flags.
////////////////////////////////////////////////////////////////////////

#define GAME_TITLE "Mega Box Crate"

#define DEBUG 1

#define SCALE 4
#define WIDTH 256
#define HEIGHT 224

#define GRAVITY -30
#define TERMINAL_VELOCITY -240

#define MAX_ENTITIES 256
#define MAX_STATIC_BODIES 20
#define MAX_BODIES 256
#define MAX_TRIGGERS 10

////////////////////////////////////////////////////////////////////////
// Shared functions.
////////////////////////////////////////////////////////////////////////

void error_and_exit(i32 code, const char *message);
f32 fsign(f32 a);
f32 fclamp(f32 a, f32 min, f32 max);

////////////////////////////////////////////////////////////////////////
// Typedefs.
////////////////////////////////////////////////////////////////////////

typedef struct hit Hit;
typedef struct aabb AABB;
typedef struct static_body Static_Body;
typedef struct trigger Trigger;
typedef struct physics_context Physics_Context;

typedef struct entity_size Entity_Size;
typedef struct entity_context Entity_Context;

typedef struct render_context Render_Context;

typedef void (*On_Collide_Function)(Hit hit);
typedef void (*On_Collide_Static_Function)(Hit hit);
typedef void (*On_Trigger_Function)(Hit);

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
};

Physics_Context *physics_setup();
void physics_tick(f32 delta_time, Entity_Context *entity_context);
Static_Body *physics_static_body_create(f32 x, f32 y, f32 half_width, f32 half_height);
Trigger *physics_trigger_create(f32 x, f32 y, f32 half_width, f32 half_height);
Hit *aabb_intersect_aabb(AABB self, AABB other);

////////////////////////////////////////////////////////////////////////
// Entity.
////////////////////////////////////////////////////////////////////////

struct entity_size {
	AABB aabb;
	vec2 velocity;
	u32 texture;
	vec2 sprite_size;
	vec2 sprite_offset;
	On_Collide_Function on_collide;
	On_Collide_Static_Function on_collide_static;
	u8 is_in_use;
	u8 is_flipped;
	u8 is_grounded;
	u8 is_kinematic;
	u8 layer_mask;
};

struct entity_context {
	AABB *aabb_array;
	vec2 *velocity_array;
	u32 *texture_array;
	vec2 *sprite_size_array;
	vec2 *sprite_offset_array;
	On_Collide_Function *on_collide_array;
	On_Collide_Static_Function *on_collide_static_array;
	u8 *is_in_use_array;
	u8 *is_flipped_array;
	u8 *is_grounded_array;
	u8 *is_kinematic_array;
	u8 *layer_mask_array;
};

Entity_Context *entity_setup();
u32 entity_create(u32 texture, f32 x, f32 y, f32 collider_half_width, f32 collider_half_height, f32 sprite_width, f32 sprite_height, f32 sprite_offset_x, f32 sprite_offset_y, u32 layer_mask);
void entity_destroy(u32 index);

////////////////////////////////////////////////////////////////////////
// Render.
////////////////////////////////////////////////////////////////////////

struct render_context {
	GLFWwindow *window;
	m4 projection;
	u32 color_texture;
	u32 shader;
	u32 square_vao;
	u32 square_vbo;
	u32 square_ebo;
	u32 line_vao;
	u32 line_vbo;
};

Render_Context *render_setup();
void render_square(f32 x, f32 y, f32 width, f32 height, vec4 color);
void render_sprite(u32 texture, vec3 position, vec2 size, u8 flipped);
void render_point(vec2 position, vec4 color);
void render_aabb(AABB aabb, vec4 color);
void render_segment(vec2 start, vec2 end, vec4 color);
void render_ray(vec2 start, vec2 direction, f32 length, vec4 color, u8 arrow);
u32 render_texture_create(const char *path);

////////////////////////////////////////////////////////////////////////
// Input output.
////////////////////////////////////////////////////////////////////////

char *read_file_into_buffer(const char *path);

////////////////////////////////////////////////////////////////////////
// User input.
////////////////////////////////////////////////////////////////////////

void input_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

#endif
