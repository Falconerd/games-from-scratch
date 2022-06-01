#include "physics_internal.h"

void physics_init_bodies(Physics_State *physics_state) {
	physics_state->body_count = 0;
	physics_state->body_max = 0;
	physics_state->bodies = malloc(MAX_BODIES * sizeof(*physics_state->bodies));
}

