#ifndef src_physics_h_INCLUDED
#define src_physics_h_INCLUDED

#include "shared.h"

#define GRAVITY 15
#define TERMINAL_VELOCITY 15

void physics_setup(Entity_Context *entity_context);
void physics_tick(f32 delta_time);

#endif

