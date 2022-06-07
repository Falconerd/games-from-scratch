#include "./game.h"
#include "./engine/render.h"
#include "./engine/util.h"

static Entity *player;
static Body *body_player;

static Body_Static *body_a;
static Body_Static *body_b;
static Body_Static *body_c;
static Body_Static *body_d;
static Body_Static *body_e;

void game_setup(Config_State *config_state, Physics_State *physics_state, Entity_State *entity_state) {
	f32 width = config_state->display_width;
	f32 height = config_state->display_height;
	u32 player_index = entity_create();
	player = &entity_state->entities[player_index];
	player->body_id = physics_body_create((vec2){500, 500}, (vec2){50, 50});
	body_player = array_list_at(physics_state->body_list, player->body_id);
	body_player->is_active = true;

	body_a = array_list_at(physics_state->body_static_list, physics_body_static_create((vec2){width*0.5-25, height-25}, (vec2){width-50, 50}));
	body_b = array_list_at(physics_state->body_static_list, physics_body_static_create((vec2){width-25, height*0.5+25}, (vec2){50, height-50}));
	body_c = array_list_at(physics_state->body_static_list, physics_body_static_create((vec2){width*0.5+25, 25}, (vec2){width-50, 50}));
	body_d = array_list_at(physics_state->body_static_list, physics_body_static_create((vec2){25, height*0.5-25}, (vec2){50, height-50}));

	body_a->is_active = true;
	body_b->is_active = true;
	body_c->is_active = true;
	body_d->is_active = true;
}

void game_handle_input(Input_State *input_state, bool *quit) {
	if (input_state->quit == KEY_STATE_PRESSED) {
		*quit = true;
	}

	f32 velx = 0;
	f32 vely = body_player->velocity[1];

	if (input_state->right == KEY_STATE_HELD || input_state->right == KEY_STATE_PRESSED) {
		velx += 8200;
	}

	if (input_state->left == KEY_STATE_HELD || input_state->left == KEY_STATE_PRESSED) {
		velx -= 8200;
	}

	vely = 0;
	if (input_state->jump == KEY_STATE_HELD || input_state->jump == KEY_STATE_PRESSED) {
		vely = 8200;
	}

	if (input_state->shoot == KEY_STATE_HELD || input_state->shoot == KEY_STATE_PRESSED) {
		vely -= 8200;
	}

	body_player->velocity[0] = velx;
	body_player->velocity[1] = vely;
}

void game_render() {
	render_aabb(&body_player->aabb, GREEN);
	render_aabb(&body_a->aabb, WHITE);
	render_aabb(&body_b->aabb, WHITE);
	render_aabb(&body_c->aabb, WHITE);
	render_aabb(&body_d->aabb, WHITE);
}
