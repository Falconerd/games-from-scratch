#include "player.h"

#define PLAYER_MOVEMENT_SPEED 40
#define PLAYER_JUMP_VELOCITY 200

void player_handle_input(Entity *player, Input_State *input_state) {
        vec2 velocity = {0, player->body->velocity[1]};

        if (input_state->left) {
            velocity[0] -= PLAYER_MOVEMENT_SPEED;
        }

        if (input_state->right) {
            velocity[0] += PLAYER_MOVEMENT_SPEED;
        }

        if (!input_state->jump && input_state->jump_last_frame > 0) {
            velocity[1] *= 0.5f;
        }

        if (input_state->jump && player->body->colliding_bottom) {
            velocity[1] = PLAYER_JUMP_VELOCITY;
        }

        player->body->velocity[0] = velocity[0];
        player->body->velocity[1] = velocity[1];
}
