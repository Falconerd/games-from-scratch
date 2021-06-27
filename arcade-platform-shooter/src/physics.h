#ifndef src_physics_h_INCLUDED
#define src_physics_h_INCLUDED

#include "shared.h"

#define MAX_OBJECTS 256
#define GRAVITY 150

typedef struct aabb {
	vec2 min;
	vec2 max;
} AABB;

typedef struct body {
	u8 fixed;
	vec2 velocity;
	AABB aabb;
} Body;

typedef struct physics_context {
	Body bodies[MAX_OBJECTS];
	u32 body_count;
} Physics_Context;

Physics_Context *physics_setup();
void physics_tick(f32 delta_time);
u32 physics_create_body(vec2 position, vec2 size);

#endif // src/physics_h_INCLUDED

