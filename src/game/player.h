#ifndef PLAYER_H
#define PLAYER_H

#include "../entity/entity.h"
#include "../input/input.h"

void player_handle_input(Entity *player, Input_State *input_state);

#endif
