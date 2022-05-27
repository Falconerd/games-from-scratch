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
	CL_BOX,
	CL_MISC
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
	// Time it took to calculate everything and render.
	f32 frame_time;

	f32 spawn_timer;

	Weapon_Type weapon_type;
	Sprite_Animation* weapon_anim;
	f32 weapon_offset_x;
	f32 weapon_offset_flipped_x;
	f32 weapon_offset_y;
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
extern Sprite_State sprite_state;

////////////////////////////////////////////////////////////////////////
// Constants.
////////////////////////////////////////////////////////////////////////

static const f32 PLAYER_SPAWN_X = WIDTH / 2;
static const f32 PLAYER_SPAWN_Y = 90;
static const f32 PLAYER_MOVEMENT_SPEED = 150;
static const f32 EXPLOSION_RADIUS = 60;
static const f32 EXPLOSION_TIME = 0.25f;
static const f32 PLAYER_JUMP_VELOCITY = 500;
static const f32 BOX_SPAWN_REGIONS[] = {
	24, 42, WIDTH - 42, HEIGHT - 84 
};
static const u32 SPAWN_REGION_COUNT = 1;

// Technically not constants - they don't change after initialisation.
static Texture TERRAIN_TEXTURE;
static Texture SPRITES_TEXTURE;
static Texture ENEMY_LARGE_TEXTURE;
static Texture ENEMY_SMALL_TEXTURE;
static Texture PLAYER_TEXTURE;
static Texture TEXTURE_PROPS;
static Texture TEXTURE_WEAPONS;
static Texture TEXTURE_SMOKE;
static Texture TEXTURE_FIRE;

static u32 SPRITE_SHEET_PLAYER;
static u32 SPRITE_SHEET_ENEMY_LARGE;
static u32 SPRITE_SHEET_ENEMY_SMALL;
static u32 SPRITE_SHEET_PROPS;
static u32 SPRITE_SHEET_WEAPONS;
static u32 SPRITE_SHEET_SMOKE;
static u32 SPRITE_SHEET_FIRE;

static u32 PLAYER_IDLE_ANIM;
static u32 PLAYER_WALK_ANIM;
static u32 SMALL_ENEMY_WALK_ANIM;
static u32 LARGE_ENEMY_WALK_ANIM;
static u32 SMALL_ANGRY_ENEMY_WALK_ANIM;
static u32 LARGE_ANGRY_ENEMY_WALK_ANIM;
static u32 BULLET_IDLE_ANIM;
static u32 BULLET_LARGE_IDLE_ANIM;
static u32 ROCKET_IDLE_ANIM;
static u32 BOX_IDLE_ANIM;
static u32 PISTOL_IDLE_ANIM;
static u32 SHOTGUN_IDLE_ANIM;
static u32 MACHINE_GUN_IDLE_ANIM;
static u32 ROCKET_LAUNCHER_IDLE_ANIM;
static u32 REVOLVER_IDLE_ANIM;
static u32 ANIM_FIRE;

static u32 SMOKE_IDLE_ANIM;

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

static void on_fire_trigger(u32 self_id, u32 trigger_id, Hit hit) {
	Entity *self = &entity_state.entity_array[self_id];

	// Make sure entities which are falling off the screen don't trigger this.
	if (self->time_to_live > 0)
		return;

	if (self_id == 0) {
		reset();
	} else if (self->layer_mask == CL_ENEMY) {
        	bool is_left_side = rand() % 100 >= 50;
		f32 spawn_x = is_left_side ? 0 - 64.0f : WIDTH + 64.0f;

		self->aabb.position[0] = spawn_x;
		self->aabb.position[1] = HEIGHT;
		
		if (self->animation_id == SMALL_ENEMY_WALK_ANIM) {
			self->animation_id = SMALL_ANGRY_ENEMY_WALK_ANIM;
			self->velocity[0] *= 1.5f;
		} else if (self->animation_id == LARGE_ENEMY_WALK_ANIM) {
			self->animation_id = LARGE_ANGRY_ENEMY_WALK_ANIM;
			self->velocity[0] *= 1.5f;
		}
	}
}

static void on_player_collide(u32 self_id, u32 other_id, Hit hit) {
	audio_sound_play(PLAYER_DEATH_SOUND);
	reset();
}

static void kill_enemy(u32 id) {
	Entity *enemy = &entity_state.entity_array[id];
	enemy->time_to_live = 3;
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
		self->is_flipped = self->is_flipped ? 0 : 1;
		self->velocity[0] = -self->velocity[0];
	} else {
		// If enemy is large, shake the screen a bit, but only once.
		if ((self->animation_id == LARGE_ENEMY_WALK_ANIM || self->animation_id == LARGE_ANGRY_ENEMY_WALK_ANIM) && self->last_velocity[1] != 0) {
			render_screen_shake_add(EXPLOSION_TIME, 0.2f);
		}
	}

}

static void spawn_projectile(Projectile_Type type, f32 x, f32 y, f32 velocity_x, f32 velocity_y, f32 time_to_live, On_Collide_Function on_collide, On_Collide_Static_Function on_collide_static) {
	Entity *player = entity_state.entity_array;
	u32 projectile_id;
	switch (type) {
	case PT_BULLET: {
		projectile_id = entity_create(x, y, 1.5f, 1.5f, 3, 3, -8.0f, -8.0f, CL_BULLET, BULLET_IDLE_ANIM);
	} break;
	case PT_BULLET_LARGE: {
		projectile_id = entity_create(x, y, 2, 2, 4, 4, -8.0f, -8.0f, CL_BULLET, BULLET_LARGE_IDLE_ANIM);
	} break;
	case PT_ROCKET: {
		projectile_id = entity_create(x, y, 4, 2.5f, 8, 5, -8.0f, -8.0f, CL_BULLET, ROCKET_IDLE_ANIM);
		Entity *projectile = &entity_state.entity_array[projectile_id];
		projectile->acceleration[0] = player->is_flipped ? -velocity_x * 0.05f : velocity_x * 0.05f;
		projectile->desired_velocity[0] = player->is_flipped ? -velocity_x : velocity_x;
		projectile->velocity[0] = 0;
		projectile->is_kinematic = 1;
		projectile->on_collide = on_collide;
		projectile->on_collide_static = on_collide_static;
		projectile->time_to_live = time_to_live;
		projectile->is_flipped = player->is_flipped;

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
	projectile->is_flipped = player->is_flipped;
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

		state.weapon_offset_x = 0;
		state.weapon_offset_flipped_x = 0;
		state.weapon_offset_y = 0;

		switch (new_weapon_type) {
		case WT_SHOTGUN: {
			state.weapon_anim = &sprite_state.sprite_animation_array[SHOTGUN_IDLE_ANIM];
			state.weapon_offset_y = -14.0f;
			state.weapon_offset_x = -10.0f;
			state.weapon_offset_flipped_x = -22.0f;
		} break;
		case WT_MACHINE_GUN: {
			state.weapon_anim = &sprite_state.sprite_animation_array[MACHINE_GUN_IDLE_ANIM];
			state.weapon_offset_y = -12.0f;
			state.weapon_offset_x = -10.0f;
			state.weapon_offset_flipped_x = -22.0f;
		} break;
		case WT_ROCKET_LAUNCHER: {
			state.weapon_anim = &sprite_state.sprite_animation_array[ROCKET_LAUNCHER_IDLE_ANIM];
			state.weapon_offset_y = -12.0f;
			state.weapon_offset_x = -8.0f;
			state.weapon_offset_flipped_x = -24.0f;
		} break;
		case WT_PISTOL: {
			state.weapon_anim = &sprite_state.sprite_animation_array[PISTOL_IDLE_ANIM];
			state.weapon_offset_y = -12.0f;
			state.weapon_offset_x = -8.0f;
			state.weapon_offset_flipped_x = -24.0f;
		} break;
		case WT_REVOLVER: {
			state.weapon_anim = &sprite_state.sprite_animation_array[REVOLVER_IDLE_ANIM];
			state.weapon_offset_y = -12.0f;
			state.weapon_offset_x = -8.0f;
			state.weapon_offset_flipped_x = -24.0f;
		} break;
		case WT_COUNT: break;
		}

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
	u32 id = entity_create(x, y, 4, 4, 8, 8, -8, -4, CL_BOX, BOX_IDLE_ANIM);
	Entity *entity = &entity_state.entity_array[id];
	entity->on_collide = on_box_collide;
}

static void reset() {
	// Destroy all entities besides the player.
	memset(&entity_state.entity_array[1], 0, (MAX_ENTITIES - 1) * sizeof(Entity));

	// Reset the player.
	Entity *player = &entity_state.entity_array[0];

	player->aabb.position[0] = PLAYER_SPAWN_X;
	player->aabb.position[1] = PLAYER_SPAWN_Y;
	player->velocity[0] = 0;
	player->velocity[1] = 0;

	// Reset state related to the player.
	state.weapon_type = WT_PISTOL;
	state.weapon_anim = &sprite_state.sprite_animation_array[PISTOL_IDLE_ANIM];
	// Refactor?: duplication
	state.weapon_offset_y = -12.0f;
	state.weapon_offset_x = -8.0f;
	state.weapon_offset_flipped_x = -24.0f;

	state.rocket_id = 0;

	state.score = 0;
	sprintf(state.score_string, "%d", state.score);

	audio_music_play(STAGE_1_THEME);
	Mix_VolumeMusic(MIX_MAX_VOLUME/2);

	spawn_box();

        u32 fire_id = entity_create(WIDTH * 0.5f, 0, 16, 32, 32, 64, -16, -32, CL_MISC, ANIM_FIRE);
	Entity *fire = &entity_state.entity_array[fire_id];
	fire->is_kinematic = true;
}

// Fix double main in Windows.
#ifdef main
#undef main
#endif

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
	TERRAIN_TEXTURE = render_texture_create("./assets/map.png");
	SPRITES_TEXTURE = render_texture_create("./assets/sprites.png");
	PLAYER_TEXTURE = render_texture_create("./assets/player.png");
	ENEMY_LARGE_TEXTURE = render_texture_create("./assets/enemy_large.png");
	ENEMY_SMALL_TEXTURE = render_texture_create("./assets/enemy_small.png");
	TEXTURE_PROPS = render_texture_create("./assets/props_16x16.png");
	TEXTURE_WEAPONS = render_texture_create("./assets/weapons.png");
	TEXTURE_SMOKE = render_texture_create("./assets/smoke.png");
	TEXTURE_FIRE = render_texture_create("./assets/fire.png");

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
	SPRITE_SHEET_PLAYER = sprite_sheet_create(PLAYER_TEXTURE, 24, 24);
	SPRITE_SHEET_ENEMY_LARGE = sprite_sheet_create(ENEMY_LARGE_TEXTURE, 40, 40);
	SPRITE_SHEET_ENEMY_SMALL = sprite_sheet_create(ENEMY_SMALL_TEXTURE, 24, 24);
	SPRITE_SHEET_PROPS = sprite_sheet_create(TEXTURE_PROPS, 16, 16);
	SPRITE_SHEET_WEAPONS = sprite_sheet_create(TEXTURE_WEAPONS, 32, 32);
	SPRITE_SHEET_SMOKE = sprite_sheet_create(TEXTURE_SMOKE, 24, 24);
	SPRITE_SHEET_FIRE = sprite_sheet_create(TEXTURE_FIRE, 32, 64);
	
	// Setup animations.
	PLAYER_WALK_ANIM = sprite_animation_create(SPRITE_SHEET_PLAYER, 7, (u8[]){1, 1, 1, 1, 1, 1, 1}, (u8[]){1, 2, 3, 4, 5, 6, 7}, (f32[]){0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f}, 1);
	PLAYER_IDLE_ANIM = sprite_animation_create(SPRITE_SHEET_PLAYER, 1, (u8[]){1}, (u8[]){0}, (f32[]){1}, 1);
	SMALL_ENEMY_WALK_ANIM = sprite_animation_create(SPRITE_SHEET_ENEMY_SMALL, 8, (u8[]){1, 1, 1, 1, 1, 1, 1, 1}, (u8[]){0, 1, 2, 3, 4, 5, 6, 7}, (f32[]){0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f}, 1);
	SMALL_ANGRY_ENEMY_WALK_ANIM = sprite_animation_create(SPRITE_SHEET_ENEMY_SMALL, 8, (u8[]){0, 0, 0, 0, 0, 0, 0, 0}, (u8[]){0, 1, 2, 3, 4, 5, 6, 7}, (f32[]){0.075f, 0.075f, 0.075f, 0.075f, 0.075f, 0.075f, 0.075f, 0.075f}, 1);
	LARGE_ENEMY_WALK_ANIM = sprite_animation_create(SPRITE_SHEET_ENEMY_LARGE, 8, (u8[]){1, 1, 1, 1, 1, 1, 1, 1}, (u8[]){0, 1, 2, 3, 4, 5, 6, 7}, (f32[]){0.10f, 0.10f, 0.10f, 0.10f, 0.10f, 0.10f, 0.10f, 0.10f}, 1);
	LARGE_ANGRY_ENEMY_WALK_ANIM = sprite_animation_create(SPRITE_SHEET_ENEMY_LARGE, 8, (u8[]){0, 0, 0, 0, 0, 0, 0, 0}, (u8[]){0, 1, 2, 3, 4, 5, 6, 7}, (f32[]){0.075f, 0.075f, 0.075f, 0.075f, 0.075f, 0.075f, 0.075f, 0.075f}, 1);
	BULLET_IDLE_ANIM = sprite_animation_create(SPRITE_SHEET_PROPS, 1, (u8[]){0}, (u8[]){0}, (f32[]){1}, 1);
	BULLET_LARGE_IDLE_ANIM = sprite_animation_create(SPRITE_SHEET_PROPS, 1, (u8[]){0}, (u8[]){1}, (f32[]){1}, 1);
	ROCKET_IDLE_ANIM = sprite_animation_create(SPRITE_SHEET_PROPS, 1, (u8[]){0}, (u8[]){2}, (f32[]){1}, 1);
	BOX_IDLE_ANIM = sprite_animation_create(SPRITE_SHEET_PROPS, 1, (u8[]){0}, (u8[]){3}, (f32[]){3}, 1);
	PISTOL_IDLE_ANIM = sprite_animation_create(SPRITE_SHEET_WEAPONS, 1, (u8[]){0}, (u8[]){3}, (f32[]){1}, 1);
	MACHINE_GUN_IDLE_ANIM = sprite_animation_create(SPRITE_SHEET_WEAPONS, 1, (u8[]){0}, (u8[]){2}, (f32[]){1}, 1);
	REVOLVER_IDLE_ANIM = sprite_animation_create(SPRITE_SHEET_WEAPONS, 1, (u8[]){1}, (u8[]){4}, (f32[]){1}, 1);
	ROCKET_LAUNCHER_IDLE_ANIM = sprite_animation_create(SPRITE_SHEET_WEAPONS, 1, (u8[]){1}, (u8[]){0}, (f32[]){1}, 1);
	SHOTGUN_IDLE_ANIM = sprite_animation_create(SPRITE_SHEET_WEAPONS, 1, (u8[]){0}, (u8[]){1}, (f32[]){1}, 1);
	SMOKE_IDLE_ANIM	= sprite_animation_create(SPRITE_SHEET_SMOKE, 1, (u8[]){0}, (u8[]){2}, (f32[]){0.15f}, 1);
	ANIM_FIRE = sprite_animation_create(SPRITE_SHEET_FIRE, 7, (u8[]){0, 0, 0, 0, 0, 0, 0}, (u8[]){0, 1, 2, 3, 4, 5, 6}, (f32[]){0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f}, 1);

	// Setup player.
	u32 player_id = entity_create(PLAYER_SPAWN_X, PLAYER_SPAWN_Y, 6, 6, 24, 24, -12, -6, CL_PLAYER, PLAYER_IDLE_ANIM);
	Entity *player = &entity_state.entity_array[player_id];
	player->on_collide = on_player_collide;

	// Setup colliders.
	{
		f32 tile_size = 32.0f;
		f32 tile_half_size = tile_size * 0.5f;

		// Top.
		physics_static_body_create(WIDTH * 0.5f, HEIGHT - tile_half_size, WIDTH, tile_size, CL_TERRAIN);

		// Sides.
		physics_static_body_create(tile_half_size, HEIGHT - 5.5f * tile_size, tile_size, 7.0f * tile_size, CL_TERRAIN);
		physics_static_body_create(WIDTH - tile_half_size, HEIGHT - 5.5f * tile_size, tile_size, 7.0f * tile_size, CL_TERRAIN);

		// Spawn platforms outside the screen.
		physics_static_body_create(-tile_half_size, HEIGHT - tile_size * 2.5f, tile_size * 3.0f, tile_size, CL_TERRAIN);
		physics_static_body_create(WIDTH + tile_half_size, HEIGHT - tile_size * 2.5f, tile_size * 3.0f, tile_size, CL_TERRAIN);

		// Bottom.
		physics_static_body_create(3.5f * tile_size, HEIGHT - 8.0f * tile_size, 7.0f * tile_size, 2.0f * tile_size, CL_TERRAIN);
		physics_static_body_create(WIDTH - 3.5f * tile_size, HEIGHT - 8.0f * tile_size, 7.0f * tile_size, 2.0f * tile_size, CL_TERRAIN);

		// Platforms.
		physics_static_body_create((64.0f + 128.0f) * 0.5f, 168.0f, 128.0f, 12.0f, CL_TERRAIN);
		physics_static_body_create(WIDTH - (64.0f + 128.0f) * 0.5f, 168.0f, 128.0f, 12.0f, CL_TERRAIN);
		physics_static_body_create(WIDTH * 0.5f, 104.0f, 288.0f, 12.0f, CL_TERRAIN);

		// Player only collision.
		physics_static_body_create(tile_half_size * 0.5f, HEIGHT - tile_size * 1.5f, tile_size, tile_size, CL_ENEMY);
		physics_static_body_create(WIDTH - tile_half_size * 0.5f, HEIGHT - tile_size * 1.5f, tile_size, tile_size, CL_ENEMY);

		// Setup fire trigger.
		Trigger *trigger = physics_trigger_create(WIDTH * 0.5f, -tile_half_size, tile_size, tile_size);
		trigger->on_trigger = on_fire_trigger;
	}

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

		Entity *player = &entity_state.entity_array[0];

		f32 horizontal_velocity = 0;
		f32 vertical_velocity = player->velocity[1];

		state.weapon_kick -= 1000 * state.delta_time;

		const u8 *keyboard_state = SDL_GetKeyboardState(NULL);

		if (keyboard_state[SDL_SCANCODE_ESCAPE]) {
			state.should_quit = 1;
		}

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
					spawn_projectile(PT_BULLET, player->aabb.position[0], player->aabb.position[1] + 4.0f, 400, frandr(-15, 15), 9, on_bullet_collide, on_bullet_collide_static);
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
						spawn_projectile(PT_BULLET, player->aabb.position[0] + (player->is_flipped ? -8.0f : 8.0f), player->aabb.position[1], vx, vy, 0.25f, on_bullet_collide, on_bullet_collide_static);
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
					spawn_projectile(PT_BULLET, player->aabb.position[0], player->aabb.position[1] + 5.0f, 300, 0, 9, on_bullet_collide, on_bullet_collide_static);
					render_screen_shake_add(0.05f, 0.03f);
				} break;
				case WT_REVOLVER: {
					audio_sound_play(REVOLVER_SOUND);
					state.shoot_timer = 0.55f;
					spawn_projectile(PT_BULLET_LARGE, player->aabb.position[0], player->aabb.position[1] + 5.0f, 300, 0, 9, on_bullet_large_collide, on_bullet_collide_static);
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
		// Update animation.
		/////////////////////////////////////////////////////////////////////
		
		if (horizontal_velocity == 0) {
			player->animation_id = PLAYER_IDLE_ANIM;
		} else {
			player->animation_id = PLAYER_WALK_ANIM;
		}
		
		/////////////////////////////////////////////////////////////////////
		// Update state.
		/////////////////////////////////////////////////////////////////////

		state.shoot_timer -= state.delta_time;

		// Spawn enemy.
		state.spawn_timer -= state.delta_time;
		if (state.spawn_timer < 0) {
			state.spawn_timer = frandr(2, 4);
			u8 health = 3;
			u32 enemy_id = 0;
			f32 speed = 100;

			bool is_small_entity = rand() % 100 > 18;
			bool is_left_side = rand() % 100 >= 50;

			f32 spawn_x = is_left_side ? 0 - 64.0f : WIDTH + 64.0f;

			if (is_small_entity) {
				enemy_id = entity_create(spawn_x, HEIGHT, 8, 8, 24, 24, -8, -8, CL_ENEMY, SMALL_ENEMY_WALK_ANIM);
			} else {
				enemy_id = entity_create(spawn_x, HEIGHT, 12, 12, 40, 40, -12, -12, CL_ENEMY, LARGE_ENEMY_WALK_ANIM);
				health = 7;
				speed = 60;
			}

			Entity *enemy = &entity_state.entity_array[enemy_id];
			enemy->health = health;
			enemy->is_flipped = !is_left_side;
			enemy->velocity[0] = is_left_side ? speed : -speed;
			enemy->on_collide_static = on_enemy_collide_static;
			enemy->time_to_live = 0;
		}

		/////////////////////////////////////////////////////////////////////
		// Render.
		/////////////////////////////////////////////////////////////////////

		// Clear screen, etc.
		glClearColor(0.0, 0.7, 0.9, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(render_state.shader);

		// Render terrain.
		// It's rendered before physics is updated as physics may have some debug rendering.
		render_sprite(TERRAIN_TEXTURE, NULL, (vec3){0, -18.0f, 0}, NULL, 0, (vec4){1, 1, 1, 1}, 0);
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

		glUseProgram(render_state.shader);

		for (u32 i = 0; i < MAX_ENTITIES; ++i) {
			Entity *entity = &entity_state.entity_array[i];
			if (!entity->is_in_use) {
				continue;
			}

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

			Sprite_Animation *sa = &sprite_state.sprite_animation_array[entity->animation_id];
			render_sprite_sheet_frame(
				sprite_state.sprite_sheet_array[sa->sprite_sheet_id],
				sa->row_coordinate_array[sa->current_frame],
				sa->column_coordinate_array[sa->current_frame],
				position,
				entity->rotation,
				entity->sprite_color,
				entity->is_flipped);


			// Render entity colliders.
#if DEBUG
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			render_aabb(entity->aabb, (vec4){0, 1, 0, 1});
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif

		}
		
		// Render player's gun.
		render_sprite_sheet_frame(
			sprite_state.sprite_sheet_array[state.weapon_anim->sprite_sheet_id],
			state.weapon_anim->row_coordinate_array[0],
			state.weapon_anim->column_coordinate_array[0],
			(f32[]){player->aabb.position[0] + (player->is_flipped ? state.weapon_offset_flipped_x : state.weapon_offset_x), player->aabb.position[1] + state.weapon_offset_y, 0},
			0.0f,
			(vec4){1, 1, 1, 1},
			player->is_flipped
		);

		// Spawn rocket smoke.
		if (state.rocket_id > 0) {
			Entity *rocket = &entity_state.entity_array[state.rocket_id];
			if (state.rocket_smoke_timer >= 0)
				state.rocket_smoke_timer -= state.delta_time;

			if (rocket->is_in_use && state.rocket_smoke_timer < 0) {
				u32 smoke_id = entity_create(rocket->aabb.position[0], rocket->aabb.position[1], 0, 0, 24, 24, -12, -12, CL_MISC, SMOKE_IDLE_ANIM);
				Entity *smoke = &entity_state.entity_array[smoke_id];
				state.rocket_smoke_timer = 0.05f;
				smoke->rotation = frandr(0, 2 * PI);
				smoke->is_kinematic = 1;
				smoke->time_to_live = frandr(0.15f, 0.6f);
				smoke->velocity[1] = frandr(-10, 10);
				smoke->velocity[0] = frandr(-10, 10);
				smoke->sprite_color_delta[3] = -0.05f;
				smoke->desired_sprite_color[3] = 0;
			}
		}

		// Update animations.
		sprite_animation_tick(state.delta_time);

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
			printf("[%d] %d\n", i, SPAWN_REGION_COUNT);
			const f32 *spawn_region = &BOX_SPAWN_REGIONS[i];
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			render_quad(spawn_region[0], spawn_region[1], spawn_region[2], spawn_region[3], (vec4){1, 1, 0.5, 0.8});
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
#endif

		glUseProgram(render_state.text_shader);
		render_text(state.score_string, WIDTH / 2, HEIGHT - 20, (vec4){1, 1, 1, 1}, 1);

#if DEBUG
		char fps[6] = {0};
		sprintf(fps, "%u", state.frame_rate);
		render_text(fps, 20, 20, (vec4){1, 1, 1, 1}, 1);
#endif

		SDL_GL_SwapWindow(render_state.window);

		// Handle capping to a set FPS.
		state.frame_time = SDL_GetTicks() - state.time_now;
		
		if (FRAME_DELAY > state.frame_time) {
			SDL_Delay(FRAME_DELAY - state.frame_time);
		}
	}
}
