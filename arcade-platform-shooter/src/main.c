#include "shared.h"

////////////////////////////////////////////////////////////////////////
// Types and context.
////////////////////////////////////////////////////////////////////////

typedef enum weapon_type {
	WT_MACHINE_GUN,
	WT_SHOTGUN,
	WT_ROCKET_LAUNCHER
} Weapon_Type;

typedef enum collision_layer {
	CL_PLAYER,
	CL_ENEMY,
	CL_BULLET,
	CL_TERRAIN,
	CL_BOX
} Collision_Layer;

typedef struct game_context {
	f32 time_now;
	f32 time_last_frame;
	f32 delta_time;

	f32 spawn_timer;
	f32 box_spawn_timer;

	Weapon_Type weapon_type;
	f32 shoot_timer;

	u8 jump_key_state;
	u8 is_box_spawned;
} Game_Context;

static Game_Context context = {0};
static Entity_Context entity_context = {0};
static Render_Context render_context = {0};
static Physics_Context physics_context = {0};

////////////////////////////////////////////////////////////////////////
// Constants.
////////////////////////////////////////////////////////////////////////

static const f32 PLAYER_SPAWN_X = 128;
static const f32 PLAYER_SPAWN_Y = 90;
static const f32 ENEMY_SPAWN_X = 128;
static const f32 ENEMY_SPAWN_Y = 224;

static u32 TERRAIN_TEXTURE;
static u32 PLAYER_TEXTURE;
static u32 BULLET_TEXTURE;
static u32 BOX_TEXTURE;
static u32 ENEMY_SMALL_TEXTURE;
static u32 ENEMY_LARGE_TEXTURE;
static u32 ENEMY_SMALL_ANGRY_TEXTURE;
static u32 ENEMY_LARGE_ANGRY_TEXTURE;

static void reset() {
	Entity *player = &entity_context.entity_array[0];
	player->aabb.position[0] = PLAYER_SPAWN_X;
	player->aabb.position[1] = PLAYER_SPAWN_Y;
	player->velocity[0] = 0;
	player->velocity[1] = 0;
	memset(&entity_context.entity_array[1], 0, (MAX_ENTITIES - 1) * sizeof(Entity));
}

static void on_fire_trigger(u32 self_id, u32 trigger_id, Hit hit) {
	Entity *self = &entity_context.entity_array[self_id];
	if (self_id == 0) {
		reset();
	} else {
		self->aabb.position[1] = ENEMY_SPAWN_Y;
		if (self->texture == ENEMY_SMALL_TEXTURE) {
			self->texture = ENEMY_SMALL_ANGRY_TEXTURE;
			self->velocity[0] *= 1.5f;
		} else if (self->texture == ENEMY_LARGE_TEXTURE) {
			self->texture = ENEMY_LARGE_ANGRY_TEXTURE;
			self->velocity[0] *= 1.5f;
		}
	}
}

static void on_player_collide(u32 self_id, u32 other_id, Hit hit) {
	reset();
}

static void on_bullet_collide(u32 self_id, u32 other_id, Hit hit) {
	entity_destroy(self_id);
	Entity *enemy = &entity_context.entity_array[other_id];
	--enemy->health;
	if (enemy->health <= 0)
		entity_destroy(other_id);
}

static void on_bullet_collide_static(u32 self_id, u32 static_body_id, Hit hit) {
	entity_destroy(self_id);
}

static void on_enemy_collide_static(u32 self_id, u32 static_body_id, Hit hit) {
	Entity *self = &entity_context.entity_array[self_id];
	if (hit.normal[0] != 0) {
		self->is_flipped = !self->is_flipped;
		self->velocity[0] = -self->velocity[0];
	}
}

static void on_box_collide(u32 self_id, u32 other_id, Hit hit) {
	Entity *self = &entity_context.entity_array[self_id];
}

static void spawn_bullet(f32 x, f32 y, f32 velocity_x, f32 velocity_y, f32 time_to_live) {
	// The player is always the first entity.
	Entity *player = entity_context.entity_array;
	u32 bullet_id = entity_create(BULLET_TEXTURE, x, y, 1.5f, 1.5f, 3, 3, -1.5f, -1.5f, CL_BULLET);
	Entity *bullet = &entity_context.entity_array[bullet_id];
	bullet->is_kinematic = 1;
	bullet->velocity[0] = player->is_flipped ? -velocity_x : velocity_x;
	bullet->velocity[1] = velocity_y;
	bullet->on_collide = on_bullet_collide;
	bullet->on_collide_static = on_bullet_collide_static;
	bullet->time_to_live = time_to_live;
}

int main(void) {
	srand(time(NULL));

	// Setup contexts.
	entity_setup(&entity_context);
	render_setup(&render_context);
	physics_setup(&physics_context);

	physics_context.mask_array[0] = 10;
	physics_context.mask_array[1] = 13;
	physics_context.mask_array[2] = 10;
	physics_context.mask_array[3] = 15;

	// @TODO: Remove me
	context.weapon_type = WT_SHOTGUN;

	// Setup textures.
	TERRAIN_TEXTURE = render_texture_create("./assets/terrain.png");
	PLAYER_TEXTURE = render_texture_create("./assets/player.png");
	BULLET_TEXTURE = render_texture_create("./assets/bullet.png");
	BOX_TEXTURE = render_texture_create("./assets/box.png");
	ENEMY_SMALL_TEXTURE = render_texture_create("./assets/enemy_small.png");
	ENEMY_LARGE_TEXTURE = render_texture_create("./assets/enemy_large.png");
	ENEMY_SMALL_ANGRY_TEXTURE = render_texture_create("./assets/enemy_small_angry.png");
	ENEMY_LARGE_ANGRY_TEXTURE = render_texture_create("./assets/enemy_large_angry.png");

	// Setup player.
	u32 player_id = entity_create(PLAYER_TEXTURE, PLAYER_SPAWN_X, PLAYER_SPAWN_Y, 4, 4, 32, 10, -16, -3.5f, CL_PLAYER);
	Entity *player = &entity_context.entity_array[player_id];
	player->on_collide = on_player_collide;

	// Setup terrain.
	// Bottom.
	physics_static_body_create(32, 18, 32, 18, CL_TERRAIN);
	physics_static_body_create(90, 10, 26, 10, CL_TERRAIN);
	physics_static_body_create(WIDTH - 32, 18, 32, 18, CL_TERRAIN);
	physics_static_body_create(WIDTH - 90, 10, 26, 10, CL_TERRAIN);
	// Top.
	physics_static_body_create(56, HEIGHT - 6.5f, 56, 6.5f, CL_TERRAIN);
	physics_static_body_create(200, HEIGHT - 6.5f, 56, 6.5f, CL_TERRAIN);
	// Left.
	physics_static_body_create(6.5f, HEIGHT / 2, 6.5f, HEIGHT / 2, CL_TERRAIN);
	// Right.
	physics_static_body_create(WIDTH - 6.5f, HEIGHT / 2, 6.5f, HEIGHT / 2, CL_TERRAIN);
	// Bottom platform.
	physics_static_body_create(128, 71, 64, 6.5f, CL_TERRAIN);
	// Top platform.
	physics_static_body_create(128, 167, 64, 6.5f, CL_TERRAIN);
	// Left platform.
	physics_static_body_create(32, 119, 32, 6.5f, CL_TERRAIN);
	// Right platform.
	physics_static_body_create(WIDTH - 32, 119, 32, 6.5f, CL_TERRAIN);
	// Top player-only collisions.
	physics_static_body_create(128, HEIGHT - 6.5f, 16, 6.5f, CL_ENEMY);

	// Setup fire trigger.
	Trigger *trigger = physics_trigger_create(128, 8, 12, 8);
	trigger->on_trigger = on_fire_trigger;

	while (!glfwWindowShouldClose(render_context.window)) {
		context.time_now = glfwGetTime();
		context.delta_time = context.time_now - context.time_last_frame;
		context.time_last_frame = context.time_now;

		// Handle user input and player movement.
		{
			f32 horizontal_velocity = 0;
			f32 vertical_velocity = player->velocity[1];

			if (glfwGetKey(render_context.window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
				glfwSetWindowShouldClose(render_context.window, GLFW_TRUE);

			// Variable jump height.
			if (glfwGetKey(render_context.window, GLFW_KEY_F) == GLFW_RELEASE && context.jump_key_state == GLFW_PRESS) {
				vertical_velocity *= 0.5f;
				context.jump_key_state = GLFW_RELEASE;
			}

			if (glfwGetKey(render_context.window, GLFW_KEY_T) == GLFW_PRESS) {
				horizontal_velocity += 80;
				player->is_flipped = 0;
			}

			if (glfwGetKey(render_context.window, GLFW_KEY_R) == GLFW_PRESS) {
				horizontal_velocity -= 80;
				player->is_flipped = 1;
			}

			// Jump.
			if (glfwGetKey(render_context.window, GLFW_KEY_F) == GLFW_PRESS) {
				if (player->is_grounded) {
					player->is_grounded = 0;
					context.jump_key_state = GLFW_PRESS;
					vertical_velocity = 500;
				}
			}

			// Shoot.
			if (glfwGetKey(render_context.window, GLFW_KEY_E) == GLFW_PRESS) {
				if (context.shoot_timer <= 0) {
					switch (context.weapon_type) {
					case WT_MACHINE_GUN: {
						context.shoot_timer = 0.05f;
						spawn_bullet(player->aabb.position[0], player->aabb.position[1], 400, 0, 9);
					} break;
					case WT_SHOTGUN: {
						context.shoot_timer = 0.75f;
						for (u32 i = 0; i < 6; ++i) {
							f32 vy = frandr(-10, 30);
							f32 vx = frandr(80, 130);
							spawn_bullet(player->aabb.position[0], player->aabb.position[1], vx, vy, 0.25f);
						}
					} break;
					case WT_ROCKET_LAUNCHER: {
						context.shoot_timer = 1.25f;
					} break;
					}
				}
			}

			player->velocity[0] = horizontal_velocity;
			player->velocity[1] = vertical_velocity;
		}

		// Update game state.
		{
			context.shoot_timer -= context.delta_time;

			// Spawn box.
			context.box_spawn_timer -= context.delta_time;
			if (context.box_spawn_timer < 0 && !context.is_box_spawned) {
				context.box_spawn_timer = frandr(2, 4);
				context.is_box_spawned = 1;
				f32 x = frandr(20, 230);
				f32 y = frandr(40, 200);
				u32 id = entity_create(BOX_TEXTURE, x, y, 4, 4, 8, 8, -4, -4, CL_BOX);
				Entity *entity = &entity_context.entity_array[id];
				entity->on_collide = on_box_collide;
			}

			// Spawn enemy.
			context.spawn_timer -= context.delta_time;
			if (context.spawn_timer < 0) {
				context.spawn_timer = frandr(2, 4);
				u8 is_flipped = (rand() % 100) > 50;
				u8 health = 3;
				u32 enemy_id = 0;
				f32 speed = 60;

				if (rand() % 100 > 25) {
					enemy_id = entity_create(ENEMY_SMALL_TEXTURE, 128, 224, 3, 3, 10, 7, -5, -3, CL_ENEMY);
				} else {
					enemy_id = entity_create(ENEMY_LARGE_TEXTURE, 128, 224, 7, 7, 14, 14, -7, -7, CL_ENEMY);
					health = 7;
					speed = 40;
				}

				Entity *enemy = &entity_context.entity_array[enemy_id];
				enemy->health = health;
				enemy->is_flipped = is_flipped;
				enemy->velocity[0] = is_flipped ? -speed : speed;
				enemy->on_collide_static = on_enemy_collide_static;
			}
		}

		// Update physics.
		physics_tick(context.delta_time, entity_context.entity_array);
		physics_cleanup();

		// Clear screen, etc.
		glfwPollEvents();
		glClearColor(0.0, 0.0, 0.0, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		// Render terrain.
		render_sprite(TERRAIN_TEXTURE, (vec3){0, 0, 0}, (vec2){256, 224}, 0);

		// Update / render entities.
		for (u32 i = 0; i < MAX_ENTITIES; ++i) {
			Entity *entity = &entity_context.entity_array[i];
			if (entity->is_in_use) {
				if (entity->time_to_live > 0) {
					entity->time_to_live -= context.delta_time;
					if (entity->time_to_live <= 0) {
						entity_destroy(i);
					}
				}
				vec3 position = {entity->aabb.position[0] + entity->sprite_offset[0],
						 entity->aabb.position[1] + entity->sprite_offset[1], 0};
				render_sprite(entity->texture, position, entity->sprite_size, entity->is_flipped);
#if DEBUG
				// Render entity colliders.
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				render_aabb(entity->aabb, (vec4){0, 1, 0, 1});
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif
			}
		}

#if DEBUG
		// Render terrain colliders.
		for (u32 i = 0; i < physics_context.static_body_array_count; ++i) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			render_aabb(physics_context.static_body_array[i].aabb, (vec4){1, 1, 1, 1});
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		// Render triggers.
		for (u32 i = 0; i < physics_context.trigger_array_count; ++i) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			render_aabb(physics_context.trigger_array[i].aabb, (vec4){1, 1, 0, 1});
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
#endif

		glfwSwapBuffers(render_context.window);
	}
}
