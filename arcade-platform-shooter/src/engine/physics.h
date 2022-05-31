#ifndef PHYSICS_H
#define PHYSICS_H

#include <stdlib.h>
#include <inttypes.h>
#include "../../deps/lib/linmath.h"
#include "./types.h"

#define MAX_BODIES 256
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
	u8 is_static;
	u8 is_active;
	u8 is_grounded;
} Body;

struct physics_state {
	u32 body_count;
	u32 body_max;
	Body *bodies;
};

u8 aabb_intersect_aabb(AABB a, AABB b);
u8 aabb_intersect_aabb_moving(AABB a, AABB b, vec2 va, vec2 vb, f32 *tfirst, f32 *tlast, f32 *nx, f32 *ny);

Physics_State *physics_init(void);
void physics_update(f32 delta_time);
u32 physics_body_create(vec2 position, vec2 size, u8 is_static);

#endif

