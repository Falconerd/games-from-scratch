#ifndef src_physics_h_INCLUDED
#define src_physics_h_INCLUDED

#include "shared.h"

#define MAX_OBJECTS 256
#define GRAVITY 15
#define TERMINAL_VELOCITY 15

typedef struct aabb {
	vec2 min;
	vec2 max;
} AABB;

typedef struct body Body;
typedef void (*Collision_Event)(Body* self, Body *other, DIRECTION direction);

struct body {
	u8 is_fixed;
	u8 is_trigger;
	u8 collision_direction;
	u8 mask;
	vec2 velocity;
	AABB aabb;
	Collision_Event on_collision_enter;
};

typedef struct physics_context {
	Body bodies[MAX_OBJECTS];
	u32 body_count;
} Physics_Context;

Physics_Context *physics_setup();
void physics_tick(f32 delta_time);
Body *physics_create_body(vec2 position, vec2 size);
void physics_reset();

#endif

