#ifndef GAME_H
#define GAME_H

#include "./engine/entity.h"
#include "./engine/physics.h"
#include "./engine/config.h"

void game_setup(Config_State *config_state, Physics_State *physics_state, Entity_State *entity_state);
void game_handle_input(Input_State *input_state, bool *quit);
void game_render();

#endif

