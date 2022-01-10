#ifndef PHYSICS_INTERNAL_H
#define PHYSICS_INTERNAL_H

#include <stdlib.h>
#include "physics.h"

void physics_collision_init(void);
void physics_body_collide_body(Physics_State *physics_state, uint32_t index);
void physics_body_collide_static(Physics_State *physics_state, uint32_t index);

#endif
