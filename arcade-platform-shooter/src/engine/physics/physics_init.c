#include "physics_internal.h"

void physics_init_bodies(Physics_State *physics_state) {
	physics_state->body_array_count = 0;
	physics_state->body_array_max = 0;
	physics_state->body_array = calloc(BODY_ARRAY_MAX, sizeof(*physics_state->body_array));

	physics_state->body_static_array_count = 0;
	physics_state->body_static_array_max = 0;
	physics_state->body_static_array = calloc(BODY_ARRAY_MAX, sizeof(*physics_state->body_static_array));
}

