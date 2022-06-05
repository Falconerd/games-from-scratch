#ifndef PHYSICS_H
#define PHYSICS_H

#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include "../../deps/lib/linmath.h"
#include "./types.h"

#define BODY_ARRAY_MAX 256
#define BODY_STATIC_ARRAY_MAX 32
#define GRAVITY -200
#define TERMINAL_VELOCITY -10000

typedef struct physics_state Physics_State;

typedef struct aabb {
	vec2 position;
	vec2 half_size;
} AABB;

typedef struct body {
	AABB aabb;
	vec2 velocity;
	bool is_active;
	bool is_grounded;
} Body;

typedef struct body_static {
	AABB aabb;
	bool is_active;
} Body_Static;

struct physics_state {
	u32 body_array_count;
	u32 body_array_max;
	Body *body_array;
	u32 body_static_array_count;
	u32 body_static_array_max;
	Body_Static *body_static_array;
};

typedef struct hit {
	f32 time;
	vec2 normal;
	vec2 delta;
	vec2 position;
} Hit;

Physics_State *physics_init(void);
void physics_update(f32 delta_time);
u32 physics_body_create(vec2 position, vec2 size);
u32 physic_body_static_create(vec2 position, vec2 size);

bool ray_intersect_aabb(vec2 p, vec2 d, AABB aabb, Hit *hit);
AABB aabb_sum(AABB a, AABB b);
#endif

