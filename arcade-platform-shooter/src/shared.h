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

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define f32 float
#define f64 double
#define i32 int32_t
#define m4 mat4x4

typedef enum direction {
	LEFT   = 1,
	RIGHT  = 2,
	TOP    = 4,
	BOTTOM = 8
} DIRECTION;

void error_and_exit(i32 code, const char *message);

#define MAX_ENTITIES 256

typedef struct aabb {
	vec2 min;
	vec2 max;
} AABB;

typedef struct body Body;
typedef void (*Collision_Event)(Body* self, Body *other, DIRECTION direction);

// Use direction enum as 1,2,4,8.
#define BODY_IS_FIXED 16
#define BODY_IS_TRIGGER 32

struct body {
	u8 flags;
	u8 mask;
	vec2 velocity;
	AABB aabb;
	Collision_Event on_collision;
};

#define ENTITY_IS_IN_USE 1
#define ENTITY_IS_ENEMY 2
#define ENTITY_IS_FLIPPED 4

typedef struct entity {
	Body body;
	u8 max_health;
	u8 health;
	u8 flags;
	u32 texture;
	vec2 sprite_offset;
	vec2 sprite_size;
} Entity;

typedef struct entity_context {
	Entity entities[MAX_ENTITIES];
} Entity_Context;

void physics_reset();

#endif

