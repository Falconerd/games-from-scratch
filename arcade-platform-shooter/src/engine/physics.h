#ifndef PHYSICS_H
#define PHYSICS_H

#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include "../../deps/lib/linmath.h"
#include "./types.h"

#define BODY_ARRAY_MAX 256
#define BODY_STATIC_ARRAY_MAX 32
#define GRAVITY -20
#define TERMINAL_VELOCITY -1000

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

u8 aabb_sweep_aabb(AABB a, AABB b, vec2 va, vec2 vb, f32 *tfirst, f32 *tlast, vec2 normal);

Physics_State *physics_init(void);
void physics_update(f32 delta_time);
u32 physics_body_create(vec2 position, vec2 size);
u32 physic_body_static_create(vec2 position, vec2 size);

#endif

