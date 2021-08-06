#include "shared.h"

////////////////////////////////////////////////////////////////////////
// Types and state.
////////////////////////////////////////////////////////////////////////

typedef enum weapon_type {
	WT_MACHINE_GUN,
	WT_SHOTGUN,
	WT_ROCKET_LAUNCHER,
	WT_PISTOL,
	WT_REVOLVER,
	WT_COUNT
} Weapon_Type;

typedef enum projectile_type {
	PT_BULLET,
	PT_BULLET_LARGE,
	PT_ROCKET,
	PT_COUNT
} Projectile_Type;

typedef enum collision_layer {
	CL_PLAYER,
	CL_ENEMY,
	CL_BULLET,
	CL_TERRAIN,
	CL_BOX
} Collision_Layer;

typedef struct game_state {
	f32 time_now;
	f32 time_last_frame;
	f32 delta_time;
	u32 frame_count;
	u32 frame_rate;
	// Used specifically for FPS calculation.
	// Not the same as time_last_frame.
	f32 previous_time;

	f32 spawn_timer;

	Weapon_Type weapon_type;
	f32 shoot_timer;

	vec2 rocket_explosion_position;
	f32 rocket_explosion_timer;

	u32 rocket_id;
	f32 rocket_smoke_timer;

	f32 weapon_kick;

	u32 score;
	char score_string[10];

	u8 should_quit;
} Game_State;

static Game_State state = {0};

extern Entity_State entity_state;
extern Render_State render_state;
extern Physics_State physics_state;
extern Input_State input_state;
extern Animation_State animation_state;

////////////////////////////////////////////////////////////////////////
// Constants.
////////////////////////////////////////////////////////////////////////

static const f32 PLAYER_SPAWN_X = WIDTH / 2;
static const f32 PLAYER_SPAWN_Y = 90;
static const f32 ENEMY_SPAWN_X = 128;
static const f32 ENEMY_SPAWN_Y = 224;
static const f32 PLAYER_MOVEMENT_SPEED = 100;
static const f32 EXPLOSION_RADIUS = 45;
static const f32 EXPLOSION_TIME = 0.25f;
static const f32 PLAYER_JUMP_VELOCITY = 400;
static const f32 BOX_SPAWN_REGIONS[] = {
	23, 173, 210, 38,
	23, 125, 210, 35,
	23, 77, 210, 35,
	140, 36, 83, 28,
	23, 36, 83, 28
};
static const u32 SPAWN_REGION_COUNT = 5;

// Technically not constants, but we cannot initialise them here and
// they never change after initialisation.
static Texture TERRAIN_TEXTURE;
static Texture SPRITES_TEXTURE;

static u32 SPRITE_SHEET;

static Mix_Music *TITLE_THEME;
static Mix_Music *STAGE_1_THEME;

static Mix_Chunk *JUMP_SOUND;
static Mix_Chunk *SHOOT_SOUND;
static Mix_Chunk *EXPLOSION_SOUND;
static Mix_Chunk *ENEMY_DEATH_SOUND;
static Mix_Chunk *REVOLVER_SOUND;
static Mix_Chunk *SHOTGUN_SOUND;
static Mix_Chunk *MACHINE_GUN_SOUND;
static Mix_Chunk *ROCKET_LAUNCHED_SOUND;
static Mix_Chunk *HURT_SOUND;
static Mix_Chunk *PLAYER_DEATH_SOUND;
static Mix_Chunk *BULLET_HIT_WALL_SOUND;
static Mix_Chunk *BOX_SOUND;

static void reset();
static void spawn_box();
/*
static void on_fire_trigger(u32 self_id, u32 trigger_id, Hit hit) {
	Entity *self = &entity_state.entity_array[self_id];
	// Make sure entities which are falling off the screen don't
	// trigger this.
	if (self->time_to_live > 0)
		return;
	if (self_id == 0) {
		reset();
	} else {
		self->aabb.position[0] = ENEMY_SPAWN_X;
		self->aabb.position[1] = ENEMY_SPAWN_Y;
		if (self->texture.id == ENEMY_SMALL_TEXTURE.id) {
			self->texture = ENEMY_SMALL_ANGRY_TEXTURE;
			self->velocity[0] *= 1.5f;
		} else if (self->texture.id == ENEMY_LARGE_TEXTURE.id) {
			self->texture = ENEMY_LARGE_ANGRY_TEXTURE;
			self->velocity[0] *= 1.5f;
		}
	}
}

static void on_player_collide(u32 self_id, u32 other_id, Hit hit) {
	// audio_sound_play(PLAYER_DEATH_SOUND);
	// reset();
}

static void kill_enemy(u32 id) {
	Entity *enemy = &entity_state.entity_array[id];
	enemy->time_to_live = 1;
	enemy->layer_mask = 5;
	enemy->velocity[1] = 100;
	audio_sound_play(ENEMY_DEATH_SOUND);
}

static void on_bullet_collide(u32 self_id, u32 other_id, Hit hit) {
	entity_destroy(self_id);
	Entity *enemy = &entity_state.entity_array[other_id];
	--enemy->health;
	if (enemy->health <= 0)
		kill_enemy(other_id);
	audio_sound_play(HURT_SOUND);
}

static void on_bullet_collide_static(u32 self_id, u32 static_body_id, Hit hit) {
	entity_destroy(self_id);
	audio_sound_play(BULLET_HIT_WALL_SOUND);
}

static void on_bullet_large_collide(u32 self_id, u32 other_id, Hit hit) {
	entity_destroy(self_id);
	Entity *enemy = &entity_state.entity_array[other_id];
	enemy->health -= 2;
	if (enemy->health <= 0)
		kill_enemy(other_id);
	audio_sound_play(HURT_SOUND);
}

static void rocket_damage(f32 pct) {
	for (u32 i = 1; i < MAX_ENTITIES; ++i) {
		Entity *entity = &entity_state.entity_array[i];
		if ((entity->layer_mask & CL_ENEMY) > 0 && !entity->is_kinematic) {
			f32 distance = vec2_sqr_dist(entity->aabb.position, state.rocket_explosion_position);
			if (distance <= EXPLOSION_RADIUS * EXPLOSION_RADIUS * pct) {
				kill_enemy(i);
			}
		}
	}
}

static void on_rocket_collide(u32 self_id, u32 other_id, Hit hit) {
	entity_destroy(self_id);
	state.rocket_explosion_position[0] = hit.position[0];
	state.rocket_explosion_position[1] = hit.position[1];
	state.rocket_explosion_timer = EXPLOSION_TIME;
	render_screen_shake_add(EXPLOSION_TIME, 1.5f);
	state.rocket_id = 0;
	audio_sound_play(EXPLOSION_SOUND);
}

static void on_enemy_collide_static(u32 self_id, u32 static_body_id, Hit hit) {
	Entity *self = &entity_state.entity_array[self_id];
	Static_Body *other = &physics_state.static_body_array[static_body_id];
	// Make sure not to flip when falling off a platform.
	if (hit.normal[0] != 0 && other->aabb.position[1] + other->aabb.half_sizes[1] > self->aabb.position[1]) {
		self->is_flipped = (u8)hit.normal[0];
		self->velocity[0] = -self->velocity[0];
	}
}

static void spawn_projectile(Projectile_Type type, f32 x, f32 y, f32 velocity_x, f32 velocity_y, f32 time_to_live, On_Collide_Function on_collide, On_Collide_Static_Function on_collide_static) {
	Entity *player = entity_state.entity_array;
	u32 projectile_id;
	switch (type) {
	case PT_BULLET: {
		projectile_id = entity_create(BULLET_SPRITE_SHEET, x, y, 1.5f, 1.5f, 3, 3, -1.5f, -1.5f, CL_BULLET);
	} break;
	case PT_BULLET_LARGE: {
		projectile_id = entity_create(LARGE_BULLET_SPRITE_SHEET, x, y, 2, 2, 4, 4, -2, -2, CL_BULLET);
	} break;
	case PT_ROCKET: {
		projectile_id = entity_create(ROCKET_SPRITE_SHEET, x, y, 4, 2.5f, 8, 5, -4, -2.5f, CL_BULLET);
		Entity *projectile = &entity_state.entity_array[projectile_id];
		projectile->acceleration[0] = player->is_flipped ? -velocity_x * 0.05f : velocity_x * 0.05f;
		projectile->desired_velocity[0] = player->is_flipped ? -velocity_x : velocity_x;
		projectile->velocity[0] = 0;
		projectile->is_kinematic = 1;
		projectile->on_collide = on_collide;
		projectile->on_collide_static = on_collide_static;
		projectile->time_to_live = time_to_live;

		state.rocket_id = projectile_id;
		state.rocket_smoke_timer = 0.01f;
		return;
	} break;
	case PT_COUNT: break;
	}
	Entity *projectile = &entity_state.entity_array[projectile_id];
	projectile->is_kinematic = 1;
	projectile->velocity[0] = player->is_flipped ? -velocity_x : velocity_x;
	projectile->velocity[1] = velocity_y;
	projectile->on_collide = on_collide;
	projectile->on_collide_static = on_collide_static;
	projectile->time_to_live = time_to_live;
}

static void on_box_collide(u32 self_id, u32 other_id, Hit hit) {
	Entity *self = &entity_state.entity_array[self_id];
	if (other_id == 0) {
		Entity *player = &entity_state.entity_array[other_id];
		Weapon_Type new_weapon_type = rand() % WT_COUNT;

		// Don't pick up the same weapon twice.
		if (state.weapon_type == new_weapon_type) {
			if (new_weapon_type == WT_COUNT - 1) {
				new_weapon_type = 0;
			} else {
				++new_weapon_type;
			}
		}

		state.weapon_type = new_weapon_type;
		entity_destroy(self_id);

		// TODO:
		// switch (new_weapon_type) {
		// case WT_SHOTGUN: player->texture = PLAYER_SHOTGUN_TEXTURE; break;
		// case WT_MACHINE_GUN: player->texture = PLAYER_MACHINE_GUN_TEXTURE; break;
		// case WT_ROCKET_LAUNCHER: player->texture = PLAYER_ROCKET_LAUNCHER_TEXTURE; break;
		// case WT_PISTOL: player->texture = PLAYER_PISTOL_TEXTURE; break;
		// case WT_REVOLVER: player->texture = PLAYER_REVOLVER_TEXTURE; break;
		// case WT_COUNT: break;
		// }

		// player->texture = PLAYER_TEXTURE;

		++state.score;
		sprintf(state.score_string, "%d", state.score);

		spawn_box();
		audio_sound_play(BOX_SOUND);
	}
}

static void spawn_box() {
	const f32 *region = &BOX_SPAWN_REGIONS[rand() % SPAWN_REGION_COUNT];
	f32 x = frandr(region[0], region[0] + region[2]);
	f32 y = frandr(region[1], region[1] + region[3]);
	u32 id = entity_create(BOX_TEXTURE, x, y, 4, 4, 8, 8, -4, -4, CL_BOX);
	Entity *entity = &entity_state.entity_array[id];
	entity->on_collide = on_box_collide;
}

*/
static void reset() {
	Entity *player = &entity_state.entity_array[0];
	player->aabb.position[0] = PLAYER_SPAWN_X;
	player->aabb.position[1] = PLAYER_SPAWN_Y;
	player->velocity[0] = 0;
	player->velocity[1] = 0;
	memset(&entity_state.entity_array[1], 0, (MAX_ENTITIES - 1) * sizeof(Entity));
	state.weapon_type = WT_PISTOL;
	// TODO:
	// player->texture = PLAYER_TEXTURE;
	// spawn_box();
	state.score = 0;
	sprintf(state.score_string, "%d", state.score);
	audio_music_play(STAGE_1_THEME);
	Mix_VolumeMusic(MIX_MAX_VOLUME/2);
}

int main(void) {
	srand(time(NULL));

	// Setup states.
	entity_setup();
	render_setup();
	physics_setup();
	input_setup();
	audio_setup();

	// Set up collision layer matrix.
	physics_state.mask_array[0] = 10;
	physics_state.mask_array[1] = 13;
	physics_state.mask_array[2] = 10;
	physics_state.mask_array[3] = 15;
	physics_state.mask_array[4] = 13;

	// Setup textures.
	TERRAIN_TEXTURE = render_texture_create("./assets/terrain.png");
	SPRITES_TEXTURE = render_texture_create("./assets/sprites.png");

	// Setup sounds.
	audio_sound_load(&JUMP_SOUND, "./assets/jump.wav");
	audio_sound_load(&SHOOT_SOUND, "./assets/shoot1.wav");
	audio_sound_load(&EXPLOSION_SOUND, "./assets/explosion.wav");
	audio_sound_load(&ENEMY_DEATH_SOUND, "./assets/enemy_death.wav");
	audio_sound_load(&REVOLVER_SOUND, "./assets/revolver.wav");
	audio_sound_load(&SHOTGUN_SOUND, "./assets/shotgun.wav");
	audio_sound_load(&MACHINE_GUN_SOUND, "./assets/machine_gun.wav");
	audio_sound_load(&ROCKET_LAUNCHED_SOUND, "./assets/rocket_launched.wav");
	audio_sound_load(&HURT_SOUND, "./assets/hurt.wav");
	audio_sound_load(&PLAYER_DEATH_SOUND, "./assets/player_death.wav");
	audio_sound_load(&BULLET_HIT_WALL_SOUND, "./assets/bullet_hit_wall.wav");
	audio_sound_load(&BOX_SOUND, "./assets/box.wav");

	audio_music_load(&TITLE_THEME, "assets/title_theme.wav");
	audio_music_load(&STAGE_1_THEME, "assets/stage_1_theme.mp3");

	// Setup sprite sheets.
	// TODO: Move up
	SPRITE_SHEET = sprite_sheet_create(SPRITES_TEXTURE, 16, 16);
	u8 rows[] = {3, 3};
	u8 cols[] = {0, 1};
	f32 fts[] = {0.25f, 0.25f};
	u32 PLAYER_WALK_ANIM = sprite_animation_create(SPRITE_SHEET, 2, rows, cols, fts, 1);

	// Setup player.
	u32 player_id = entity_create(PLAYER_SPAWN_X, PLAYER_SPAWN_Y, 4, 4, 16, 10, -8, -4, CL_PLAYER);
	Entity *player = &entity_state.entity_array[player_id];
	player->animation_id = PLAYER_WALK_ANIM;
	// player->on_collide = on_player_collide;

	// Setup terrain.
	{
		f32 platform_half_height = 13.0 / 2.0;
		f32 side_edge_half_width = 13.0  / 2.0;
		f32 side_platform_half_width = 96.0 / 2.0;
		f32 middle_platform_half_width = 192 / 2.0;
		f32 top_edge_half_width = 176.0 / 2.0;
		f32 bottom_big_half_width = 112.0 / 2.0;
		f32 bottom_big_half_height = 36.0 / 2.0;
		f32 bottom_small_half_width = 180.0 / 2.0;
		f32 bottom_small_half_height = 20.0 / 2.0;

		f32 side_platform_y = 107 + platform_half_height;
		f32 middle_bottom_platform_y = 65 + platform_half_height;
		f32 middle_top_platform_y = 155 + platform_half_height;
		f32 middle_platform_x = 96 + middle_platform_half_width;

		// Bottom.
		physics_static_body_create(bottom_big_half_width, bottom_big_half_height, bottom_big_half_width, bottom_big_half_height, CL_TERRAIN);
		physics_static_body_create(bottom_small_half_width, bottom_small_half_height, bottom_small_half_width, bottom_small_half_height, CL_TERRAIN);
		physics_static_body_create(WIDTH - bottom_big_half_width, bottom_big_half_height, bottom_big_half_width, bottom_big_half_height, CL_TERRAIN);
		physics_static_body_create(WIDTH - bottom_small_half_width, bottom_small_half_height, bottom_small_half_width, bottom_small_half_height, CL_TERRAIN);

		// Top.
		physics_static_body_create(top_edge_half_width, HEIGHT - platform_half_height, top_edge_half_width, platform_half_height, CL_TERRAIN);
		physics_static_body_create(WIDTH - top_edge_half_width, HEIGHT - platform_half_height, top_edge_half_width, platform_half_height, CL_TERRAIN);

		// Left.
		physics_static_body_create(side_edge_half_width, HEIGHT / 2.0, side_edge_half_width, HEIGHT / 2.0, CL_TERRAIN);

		// Right.
		physics_static_body_create(WIDTH - side_edge_half_width, HEIGHT / 2.0, side_edge_half_width, HEIGHT / 2.0, CL_TERRAIN);

		// Bottom platform.
		physics_static_body_create(middle_platform_x, middle_bottom_platform_y, middle_platform_half_width, platform_half_height, CL_TERRAIN);

		// Top platform.
		physics_static_body_create(middle_platform_x, middle_top_platform_y, middle_platform_half_width, platform_half_height, CL_TERRAIN);

		// Left platform.
		physics_static_body_create(side_platform_half_width, side_platform_y, side_platform_half_width, platform_half_height, CL_TERRAIN);

		// Right platform.
		physics_static_body_create(WIDTH - side_platform_half_width, side_platform_y, side_platform_half_width, platform_half_height, CL_TERRAIN);

		// Top player-only collisions.
		physics_static_body_create(WIDTH / 2.0, HEIGHT - platform_half_height, 16, 6.5f, CL_ENEMY);
	}

	// Setup fire trigger.
	Trigger *trigger = physics_trigger_create(WIDTH / 2.0, 8, 12, 8);
	// trigger->on_trigger = on_fire_trigger;

	reset();

	state.previous_time = (f32)SDL_GetTicks();

	while (!state.should_quit) {
		state.time_now = (f32)SDL_GetTicks();
		state.delta_time = (state.time_now - state.time_last_frame) / 1000;
		state.time_last_frame = state.time_now;
		state.frame_count++;
		if (state.time_now - state.previous_time >= 1000.0f) {
			state.frame_rate = state.frame_count;
			state.frame_count = 0;
			state.previous_time = state.time_now;
		}

		/////////////////////////////////////////////////////////////////////
		// Handle input.
		/////////////////////////////////////////////////////////////////////
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				exit(0);
			default:
				break;
			}
		}
/*

		Entity *player = &entity_state.entity_array[0];

		f32 horizontal_velocity = 0;
		f32 vertical_velocity = player->velocity[1];

		state.weapon_kick -= 1000 * state.delta_time;
*/
		const u8 *keyboard_state = SDL_GetKeyboardState(NULL);

		if (keyboard_state[SDL_SCANCODE_ESCAPE]) {
			state.should_quit = 1;
		}
/*
		if (keyboard_state[input_state.right]) {
			horizontal_velocity += PLAYER_MOVEMENT_SPEED;
			player->is_flipped = 0;
		}

		if (keyboard_state[input_state.left]) {
			horizontal_velocity -= PLAYER_MOVEMENT_SPEED;
			player->is_flipped = 1;
		}

		if (!keyboard_state[input_state.jump] && input_state.jump_key_was_pressed) {
			vertical_velocity *= 0.5f;
			input_state.jump_key_was_pressed = 0;
		}

		if (keyboard_state[input_state.jump]) {
			if (player->is_grounded) {
				player->is_grounded = 0;
				input_state.jump_key_was_pressed = 1;
				vertical_velocity = PLAYER_JUMP_VELOCITY;
				audio_sound_play(JUMP_SOUND);
			}
		}

		if (keyboard_state[SDL_SCANCODE_E]) {
			if (state.shoot_timer <= 0) {
				switch (state.weapon_type) {
				case WT_MACHINE_GUN: {
					audio_sound_play(MACHINE_GUN_SOUND);
					state.shoot_timer = 0.05f;
					spawn_projectile(PT_BULLET, player->aabb.position[0], player->aabb.position[1], 400, frandr(-15, 15), 9, on_bullet_collide, on_bullet_collide_static);
					state.weapon_kick = 100;
					render_screen_shake_add(0.05f, 0.15f);
				} break;
				case WT_SHOTGUN: {
					audio_sound_play(SHOTGUN_SOUND);
					state.shoot_timer = 0.75f;
					render_screen_shake_add(0.1f, 0.75f);
					for (u32 i = 0; i < 15; ++i) {
						f32 vy = frandr(-35, 35);
						f32 vx = frandr(150, 210);
						spawn_projectile(PT_BULLET, player->aabb.position[0], player->aabb.position[1], vx, vy, 0.25f, on_bullet_collide, on_bullet_collide_static);
					}
				} break;
				case WT_ROCKET_LAUNCHER: {
					audio_sound_play(ROCKET_LAUNCHED_SOUND);
					state.shoot_timer = 1.25f;
					spawn_projectile(PT_ROCKET, player->aabb.position[0], player->aabb.position[1], 200, 0, 9, on_rocket_collide, on_rocket_collide);
				} break;
				case WT_PISTOL: {
					audio_sound_play(SHOOT_SOUND);
					state.shoot_timer = 0.25f;
					spawn_projectile(PT_BULLET, player->aabb.position[0], player->aabb.position[1], 300, 0, 9, on_bullet_collide, on_bullet_collide_static);
					render_screen_shake_add(0.05f, 0.03f);
				} break;
				case WT_REVOLVER: {
					audio_sound_play(REVOLVER_SOUND);
					state.shoot_timer = 0.55f;
					spawn_projectile(PT_BULLET_LARGE, player->aabb.position[0], player->aabb.position[1], 300, 0, 9, on_bullet_large_collide, on_bullet_collide_static);
					render_screen_shake_add(0.1f, 0.75f);
				} break;
				case WT_COUNT: break;
				}
			}
		}

		if (state.weapon_kick >= 0) {
			horizontal_velocity = player->is_flipped ? state.weapon_kick : -state.weapon_kick;
		}

		player->velocity[0] = horizontal_velocity;
		player->velocity[1] = vertical_velocity;

		/////////////////////////////////////////////////////////////////////
		// Update state.
		/////////////////////////////////////////////////////////////////////

		state.shoot_timer -= state.delta_time;

		// Spawn enemy.
		state.spawn_timer -= state.delta_time;
		if (state.spawn_timer < 0) {
			state.spawn_timer = frandr(2, 4);
			u8 is_flipped = (rand() % 100) > 50;
			u8 health = 3;
			u32 enemy_id = 0;
			f32 speed = 60;

			if (rand() % 100 > 18) {
				enemy_id = entity_create(SPRITE_SHEET, WIDTH / 2.0, HEIGHT, 3, 3, 10, 7, -5, -3, CL_ENEMY);
			} else {
				enemy_id = entity_create(SPRITE_SHEET, WIDTH / 2.0, HEIGHT, 7, 7, 14, 14, -7, -7, CL_ENEMY);
				health = 7;
				speed = 40;
			}

			Entity *enemy = &entity_state.entity_array[enemy_id];
			enemy->health = health;
			enemy->is_flipped = is_flipped;
			enemy->velocity[0] = is_flipped ? -speed : speed;
			enemy->on_collide_static = on_enemy_collide_static;
			enemy->time_to_live = 0;
		}

		/////////////////////////////////////////////////////////////////////
		// Render.
		/////////////////////////////////////////////////////////////////////
*/
		// Clear screen, etc.
		glClearColor(0.0, 0.7, 0.9, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(render_state.shader);

		// Render terrain.
		// It's rendered before physics is updated as physics may have some debug rendering.
		render_sprite(TERRAIN_TEXTURE, NULL, (vec3){0, 0, 0}, NULL, 0, (vec4){1, 1, 1, 1}, 0);
/*
		// Update physics.
		physics_tick(state.delta_time, entity_state.entity_array);
		physics_cleanup();

		render_screen_shake(state.delta_time);

		// Render explosion.
		if (state.rocket_explosion_timer > 0) {
			glUseProgram(render_state.circle_shader);
			f32 pct = 1.0f - state.rocket_explosion_timer / EXPLOSION_TIME;
			render_circle(state.rocket_explosion_position[0],
			              state.rocket_explosion_position[1],
			              EXPLOSION_RADIUS * pct, (vec4){1, 1, 1, 1});
			state.rocket_explosion_timer -= state.delta_time;
			rocket_damage(pct);
		}
*/
		glUseProgram(render_state.shader);
		for (u32 i = 0; i < MAX_ENTITIES; ++i) {
			Entity *entity = &entity_state.entity_array[i];
			if (!entity->is_in_use) {
				continue;
			}

			v3 position = {entity->aabb.position[0] + entity->sprite_offset[0],
			               entity->aabb.position[1] + entity->sprite_offset[1], 0};

	             	Sprite_Animation *sa = &animation_state.sprite_animation_array[entity->animation_id];
			render_sprite_sheet_frame(
				animation_state.sprite_sheet_array[sa->sprite_sheet_id],
				sa->row_coordinate_array[sa->current_frame],
				sa->column_coordinate_array[sa->current_frame],
				position,
				0,
				(v4){1, 1, 1, 1},
				entity->is_flipped);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			render_aabb(entity->aabb, (vec4){0, 1, 0, 1});
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		// Update animations.
		sprite_animation_tick(state.delta_time);
/*

		// Spawn rocket smoke.
		if (state.rocket_id > 0) {
			Entity *rocket = &entity_state.entity_array[state.rocket_id];
			if (state.rocket_smoke_timer >= 0)
				state.rocket_smoke_timer -= state.delta_time;

			if (rocket->is_in_use && state.rocket_smoke_timer < 0) {
				u32 smoke_id = entity_create(SMOKE_TEXTURE, rocket->aabb.position[0], rocket->aabb.position[1], 0, 0, 10, 10, -5, -5, 5);
				Entity *smoke = &entity_state.entity_array[smoke_id];
				state.rocket_smoke_timer = 0.05f;
				smoke->rotation = frandr(0, 2 * PI);
				smoke->is_kinematic = 1;
				smoke->time_to_live = frandr(0.8f, 1.2f);
				smoke->velocity[1] = frandr(-10, 10);
				smoke->velocity[0] = frandr(-10, 10);
				smoke->sprite_color_delta[3] = -0.05f;
				smoke->desired_sprite_color[3] = 0;
			}
		}

		// Update / render entities.
		for (u32 i = 0; i < MAX_ENTITIES; ++i) {
			Entity *entity = &entity_state.entity_array[i];
			if (entity->is_in_use) {
				if (entity->time_to_live > 0) {
					if (!entity->is_kinematic)
						entity->rotation += state.delta_time * 10;
					entity->time_to_live -= state.delta_time;
					if (entity->time_to_live <= 0) {
						entity_destroy(i);
					}
				}

				// Update sprite color.
				for (u32 j = 0; j < 4; ++j) {
					entity->sprite_color[j] += entity->sprite_color_delta[j];
					if (entity->sprite_color_delta[j] != 0 && fabs(entity->sprite_color[j]) < fabs(entity->desired_sprite_color[j])) {
						entity->sprite_color[j] = entity->desired_sprite_color[j];
					}
				}
				vec3 position = {entity->aabb.position[0] + entity->sprite_offset[0],
						 entity->aabb.position[1] + entity->sprite_offset[1], 0};
				f32 uvs[] = {
					1.0f, 1.0f,
					1.0f, 0.0f,
					0.5f, 0.0f,
					0.5f, 1.0f
				};
				render_sprite(entity->sprite_sheet.texture, NULL, position, uvs, entity->rotation, entity->sprite_color, entity->is_flipped);

#if DEBUG
				// Render entity colliders.
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				render_aabb(entity->aabb, (vec4){0, 1, 0, 1});

				render_quad(entity->aabb.position[0] - entity->sprite_size[0] * 0.5,
				            entity->aabb.position[1] - entity->sprite_size[1] * 0.5,
				            entity->sprite_size[0],
				            entity->sprite_size[1],
				            (vec4){1, 1, 0, 0.5});
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif
			}
		}

#if DEBUG
		// Render terrain colliders.
		for (u32 i = 0; i < physics_state.static_body_array_count; ++i) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			render_aabb(physics_state.static_body_array[i].aabb, (vec4){1, 1, 1, 1});
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		// Render triggers.
		for (u32 i = 0; i < physics_state.trigger_array_count; ++i) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			render_aabb(physics_state.trigger_array[i].aabb, (vec4){1, 1, 0, 1});
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		// Render spawn regions.
		for (u32 i = 0; i < SPAWN_REGION_COUNT; ++i) {
			const f32 *spawn_region = &BOX_SPAWN_REGIONS[i];
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			render_quad(spawn_region[0], spawn_region[1], spawn_region[2], spawn_region[3], (vec4){1, 1, 0.5, 0.1});
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
#endif

		glUseProgram(render_state.text_shader);
		render_text(state.score_string, 128, 194, (vec4){1, 1, 1, 1}, 1);

		char fps[6] = {0};
		sprintf(fps, "%u", state.frame_rate);
		
		render_text(fps, 20, 20, (vec4){1, 1, 1, 1}, 1);
		*/
		SDL_GL_SwapWindow(render_state.window);
	}
}
