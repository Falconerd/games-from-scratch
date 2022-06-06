#include "physics_internal.h"

void physics_init_bodies(Physics_State *physics_state) {
	physics_state->body_list = array_list_init(sizeof(Body), BODY_ARRAY_LENGTH);
	physics_state->body_static_list = array_list_init(sizeof(Body_Static), BODY_STATIC_ARRAY_LENGTH);
}

