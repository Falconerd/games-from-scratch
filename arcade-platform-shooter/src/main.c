#include "shared.h"

////////////////////////////////////////////////////////////////////////
// Types and context.
////////////////////////////////////////////////////////////////////////

typedef struct spawn_region {
	f32 x;
	f32 y;
	f32 w;
	f32 h;
} Spawn_Region;

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

typedef struct game_context {
	f32 time_now;
	f32 time_last_frame;
	f32 delta_time;

	f32 spawn_timer;

	Weapon_Type weapon_type;
	f32 shoot_timer;

	vec2 rocket_explosion_position;
	f32 rocket_explosion_timer;

	f32 weapon_kick;

	u32 rocket_id;
	f32 rocket_smoke_timer;

	u8 jump_key_state;
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
static const f32 PLAYER_MOVEMENT_SPEED = 100;
static const f32 EXPLOSION_RADIUS = 45;
static const f32 EXPLOSION_TIME = 0.25f;
static const f32 PLAYER_JUMP_VELOCITY = 400;

static u32 TERRAIN_TEXTURE;
static u32 PLAYER_TEXTURE;
static u32 PLAYER_SHOTGUN_TEXTURE;
static u32 PLAYER_MACHINE_GUN_TEXTURE;
static u32 PLAYER_ROCKET_LAUNCHER_TEXTURE;
static u32 PLAYER_PISTOL_TEXTURE;
static u32 PLAYER_REVOLVER_TEXTURE;
static u32 BULLET_TEXTURE;
static u32 BULLET_LARGE_TEXTURE;
static u32 ROCKET_TEXTURE;
static u32 BOX_TEXTURE;
static u32 SMOKE_TEXTURE;
static u32 ENEMY_SMALL_TEXTURE;
static u32 ENEMY_LARGE_TEXTURE;
static u32 ENEMY_SMALL_ANGRY_TEXTURE;
static u32 ENEMY_LARGE_ANGRY_TEXTURE;

static Spawn_Region BOX_SPAWN_REGIONS[] = {
	{23, 173, 210, 38},
	{23, 125, 210, 35},
	{23, 77, 210, 35},
	{140, 36, 83, 28},
	{23, 36, 83, 28}
};
static u32 SPAWN_REGION_COUNT = 5;

static void reset();
static void spawn_box();

static void on_fire_trigger(u32 self_id, u32 trigger_id, Hit hit) {
	Entity *self = &entity_context.entity_array[self_id];
	// Make sure entities which are falling off the screen don't
	// trigger this.
	if (self->time_to_live > 0)
		return;
	if (self_id == 0) {
		reset();
	} else {
		self->aabb.position[0] = ENEMY_SPAWN_X;
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
	// reset();
}

static void kill_enemy(u32 id) {
	Entity *enemy = &entity_context.entity_array[id];
	enemy->time_to_live = 1;
	enemy->layer_mask = 5;
	enemy->velocity[1] = 100;
}

static void on_bullet_collide(u32 self_id, u32 other_id, Hit hit) {
	entity_destroy(self_id);
	Entity *enemy = &entity_context.entity_array[other_id];
	--enemy->health;
	if (enemy->health <= 0)
		kill_enemy(other_id);
}

static void on_bullet_collide_static(u32 self_id, u32 static_body_id, Hit hit) {
	entity_destroy(self_id);
}

static void on_bullet_large_collide(u32 self_id, u32 other_id, Hit hit) {
	Entity *enemy = &entity_context.entity_array[other_id];
	enemy->health -= 2;
	if (enemy->health <= 0)
		kill_enemy(other_id);
}

static void rocket_damage(f32 pct) {
	for (u32 i = 1; i < MAX_ENTITIES; ++i) {
		Entity *entity = &entity_context.entity_array[i];
		if ((entity->layer_mask & CL_ENEMY) > 0 && !entity->is_kinematic) {
			f32 distance = vec2_sqr_dist(entity->aabb.position, context.rocket_explosion_position);
			if (distance <= EXPLOSION_RADIUS * EXPLOSION_RADIUS * pct) {
				kill_enemy(i);
			}
		}
	}
}

static void on_rocket_collide(u32 self_id, u32 other_id, Hit hit) {
	entity_destroy(self_id);
	context.rocket_explosion_position[0] = hit.position[0];
	context.rocket_explosion_position[1] = hit.position[1];
	context.rocket_explosion_timer = EXPLOSION_TIME;
	render_screen_shake_add(EXPLOSION_TIME, 1.5f);
	context.rocket_id = 0;
}

static void on_enemy_collide_static(u32 self_id, u32 static_body_id, Hit hit) {
	Entity *self = &entity_context.entity_array[self_id];
	Static_Body *other = &physics_context.static_body_array[static_body_id];
	// Make sure not to flip when falling off a platform.
	if (hit.normal[0] != 0 && other->aabb.position[1] + other->aabb.half_sizes[1] > self->aabb.position[1]) {
		self->is_flipped = (u8)hit.normal[0];
		self->velocity[0] = -self->velocity[0];
	}
}

static void spawn_projectile(Projectile_Type type, f32 x, f32 y, f32 velocity_x, f32 velocity_y, f32 time_to_live, On_Collide_Function on_collide, On_Collide_Static_Function on_collide_static) {
	Entity *player = entity_context.entity_array;
	u32 projectile_id;
	switch (type) {
	case PT_BULLET: {
		projectile_id = entity_create(BULLET_TEXTURE, x, y, 1.5f, 1.5f, 3, 3, -1.5f, -1.5f, CL_BULLET);
	} break;
	case PT_BULLET_LARGE: {
		projectile_id = entity_create(BULLET_LARGE_TEXTURE, x, y, 2, 2, 4, 4, -2, -2, CL_BULLET);
	} break;
	case PT_ROCKET: {
		projectile_id = entity_create(ROCKET_TEXTURE, x, y, 4, 2.5f, 8, 5, -4, -2.5f, CL_BULLET);
		Entity *projectile = &entity_context.entity_array[projectile_id];
		projectile->acceleration[0] = player->is_flipped ? -velocity_x * 0.05f : velocity_x * 0.05f;
		projectile->desired_velocity[0] = player->is_flipped ? -velocity_x : velocity_x;
		projectile->velocity[0] = 0;
		projectile->is_kinematic = 1;
		projectile->on_collide = on_collide;
		projectile->on_collide_static = on_collide_static;
		projectile->time_to_live = time_to_live;

		context.rocket_id = projectile_id;
		context.rocket_smoke_timer = 0.01f;
		return;
	} break;
	}
	Entity *projectile = &entity_context.entity_array[projectile_id];
	projectile->is_kinematic = 1;
	projectile->velocity[0] = player->is_flipped ? -velocity_x : velocity_x;
	projectile->velocity[1] = velocity_y;
	projectile->on_collide = on_collide;
	projectile->on_collide_static = on_collide_static;
	projectile->time_to_live = time_to_live;
}

static void on_box_collide(u32 self_id, u32 other_id, Hit hit) {
	Entity *self = &entity_context.entity_array[self_id];
	if (other_id == 0) {
		Entity *player = &entity_context.entity_array[other_id];
		Weapon_Type new_weapon_type = rand() % WT_COUNT;

		// Don't pick up the same weapon twice.
		if (context.weapon_type == new_weapon_type) {
			if (new_weapon_type == WT_COUNT - 1) {
				new_weapon_type = 0;
			} else {
				++new_weapon_type;
			}
		}

		context.weapon_type = new_weapon_type;
		entity_destroy(self_id);

		switch (new_weapon_type) {
		case WT_SHOTGUN: player->texture = PLAYER_SHOTGUN_TEXTURE; break;
		case WT_MACHINE_GUN: player->texture = PLAYER_MACHINE_GUN_TEXTURE; break;
		case WT_ROCKET_LAUNCHER: player->texture = PLAYER_ROCKET_LAUNCHER_TEXTURE; break;
		case WT_PISTOL: player->texture = PLAYER_PISTOL_TEXTURE; break;
		case WT_REVOLVER: player->texture = PLAYER_REVOLVER_TEXTURE; break;
		}

		spawn_box();
	}
}

static void spawn_box() {
	Spawn_Region region = BOX_SPAWN_REGIONS[rand() % SPAWN_REGION_COUNT];
	f32 x = frandr(region.x, region.x + region.w);
	f32 y = frandr(region.y, region.y + region.h);
	u32 id = entity_create(BOX_TEXTURE, x, y, 4, 4, 8, 8, -4, -4, CL_BOX);
	Entity *entity = &entity_context.entity_array[id];
	entity->on_collide = on_box_collide;
}

static void reset() {
	Entity *player = &entity_context.entity_array[0];
	player->aabb.position[0] = PLAYER_SPAWN_X;
	player->aabb.position[1] = PLAYER_SPAWN_Y;
	player->velocity[0] = 0;
	player->velocity[1] = 0;
	memset(&entity_context.entity_array[1], 0, (MAX_ENTITIES - 1) * sizeof(Entity));
	context.weapon_type = WT_PISTOL;
	player->texture = PLAYER_PISTOL_TEXTURE;
	spawn_box();
}

int main(void) {
	srand(time(NULL));

	// Setup contexts.
	entity_setup(&entity_context);
	render_setup(&render_context);
	physics_setup(&physics_context);

	// Set up collision layer matrix.
	physics_context.mask_array[0] = 10;
	physics_context.mask_array[1] = 13;
	physics_context.mask_array[2] = 10;
	physics_context.mask_array[3] = 15;
	physics_context.mask_array[4] = 13;

	// Setup textures.
	TERRAIN_TEXTURE = render_texture_create("./assets/terrain.png");
	PLAYER_TEXTURE = render_texture_create("./assets/player.png");
	PLAYER_SHOTGUN_TEXTURE = render_texture_create("./assets/player_shotgun.png");
	PLAYER_MACHINE_GUN_TEXTURE = render_texture_create("./assets/player_machine_gun.png");
	PLAYER_ROCKET_LAUNCHER_TEXTURE = render_texture_create("./assets/player_bazooka.png");
	PLAYER_PISTOL_TEXTURE = render_texture_create("./assets/player_pistol.png");
	PLAYER_REVOLVER_TEXTURE = render_texture_create("./assets/player_revolver.png");
	BULLET_TEXTURE = render_texture_create("./assets/bullet.png");
	BULLET_LARGE_TEXTURE = render_texture_create("./assets/bullet_large.png");
	ROCKET_TEXTURE = render_texture_create("./assets/rocket.png");
	BOX_TEXTURE = render_texture_create("./assets/box.png");
	SMOKE_TEXTURE = render_texture_create("./assets/smoke.png");
	ENEMY_SMALL_TEXTURE = render_texture_create("./assets/enemy_small.png");
	ENEMY_LARGE_TEXTURE = render_texture_create("./assets/enemy_large.png");
	ENEMY_SMALL_ANGRY_TEXTURE = render_texture_create("./assets/enemy_small_angry.png");
	ENEMY_LARGE_ANGRY_TEXTURE = render_texture_create("./assets/enemy_large_angry.png");

	// Setup player.
	u32 player_id = entity_create(PLAYER_TEXTURE, PLAYER_SPAWN_X, PLAYER_SPAWN_Y, 4, 4, 32, 10, -16, -3.5f, CL_PLAYER);
	Entity *player = &entity_context.entity_array[player_id];
	player->on_collide = on_player_collide;

	context.weapon_type = WT_ROCKET_LAUNCHER;
	player->texture = PLAYER_ROCKET_LAUNCHER_TEXTURE;

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
	physics_static_body_create(128, 70.5f, 64, 6.5f, CL_TERRAIN);
	// Top platform.
	physics_static_body_create(128, 166.5f, 64, 6.5f, CL_TERRAIN);
	// Left platform.
	physics_static_body_create(32, 118.5f, 32, 6.5f, CL_TERRAIN);
	// Right platform.
	physics_static_body_create(WIDTH - 32, 118.5f, 32, 6.5f, CL_TERRAIN);
	// Top player-only collisions.
	physics_static_body_create(128, HEIGHT - 6.5f, 16, 6.5f, CL_ENEMY);

	// Setup fire trigger.
	Trigger *trigger = physics_trigger_create(128, 8, 12, 8);
	trigger->on_trigger = on_fire_trigger;

	reset();

	while (!glfwWindowShouldClose(render_context.window)) {
		context.time_now = glfwGetTime();
		context.delta_time = context.time_now - context.time_last_frame;
		context.time_last_frame = context.time_now;

		// Handle user input and player movement.
		{
			f32 horizontal_velocity = 0;
			f32 vertical_velocity = player->velocity[1];

			context.weapon_kick -= 1000 * context.delta_time;

			if (glfwGetKey(render_context.window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
				glfwSetWindowShouldClose(render_context.window, GLFW_TRUE);

			// Variable jump height.
			if (glfwGetKey(render_context.window, GLFW_KEY_F) == GLFW_RELEASE && context.jump_key_state == GLFW_PRESS) {
				vertical_velocity *= 0.5f;
				context.jump_key_state = GLFW_RELEASE;
			}

			if (glfwGetKey(render_context.window, GLFW_KEY_T) == GLFW_PRESS) {
				horizontal_velocity += PLAYER_MOVEMENT_SPEED;
				player->is_flipped = 0;
			}

			if (glfwGetKey(render_context.window, GLFW_KEY_R) == GLFW_PRESS) {
				horizontal_velocity -= PLAYER_MOVEMENT_SPEED;
				player->is_flipped = 1;
			}

			// Jump.
			if (glfwGetKey(render_context.window, GLFW_KEY_F) == GLFW_PRESS) {
				if (player->is_grounded) {
					player->is_grounded = 0;
					context.jump_key_state = GLFW_PRESS;
					vertical_velocity = PLAYER_JUMP_VELOCITY;
				}
			}

			// Shoot.
			if (glfwGetKey(render_context.window, GLFW_KEY_E) == GLFW_PRESS) {
				if (context.shoot_timer <= 0) {
					switch (context.weapon_type) {
					case WT_MACHINE_GUN: {
						context.shoot_timer = 0.05f;
						spawn_projectile(PT_BULLET, player->aabb.position[0], player->aabb.position[1], 400, frandr(-15, 15), 9, on_bullet_collide, on_bullet_collide_static);
						context.weapon_kick = 100;
						render_screen_shake_add(0.05f, 0.15f);
					} break;
					case WT_SHOTGUN: {
						context.shoot_timer = 0.75f;
						render_screen_shake_add(0.1f, 0.75f);
						for (u32 i = 0; i < 15; ++i) {
							f32 vy = frandr(-35, 35);
							f32 vx = frandr(150, 210);
							spawn_projectile(PT_BULLET, player->aabb.position[0], player->aabb.position[1], vx, vy, 0.25f, on_bullet_collide, on_bullet_collide_static);
						}
					} break;
					case WT_ROCKET_LAUNCHER: {
						context.shoot_timer = 1.25f;
						spawn_projectile(PT_ROCKET, player->aabb.position[0], player->aabb.position[1], 200, 0, 9, on_rocket_collide, on_rocket_collide);
					} break;
					case WT_PISTOL: {
						context.shoot_timer = 0.25f;
						spawn_projectile(PT_BULLET, player->aabb.position[0], player->aabb.position[1], 300, 0, 9, on_bullet_collide, on_bullet_collide_static);
						render_screen_shake_add(0.05f, 0.03f);
					} break;
					case WT_REVOLVER: {
						context.shoot_timer = 0.55f;
						spawn_projectile(PT_BULLET_LARGE, player->aabb.position[0], player->aabb.position[1], 300, 0, 9, on_bullet_large_collide, on_bullet_collide_static);
						render_screen_shake_add(0.1f, 0.75f);
					} break;
					}
				}
			}

			if (context.weapon_kick >= 0) {
				horizontal_velocity = player->is_flipped ? context.weapon_kick : -context.weapon_kick;
			}

			player->velocity[0] = horizontal_velocity;
			player->velocity[1] = vertical_velocity;
		}

		// Update game state.
		{
			context.shoot_timer -= context.delta_time;

			// Spawn enemy.
			context.spawn_timer -= context.delta_time;
			if (context.spawn_timer < 0) {
				context.spawn_timer = frandr(2, 4);
				u8 is_flipped = (rand() % 100) > 50;
				u8 health = 3;
				u32 enemy_id = 0;
				f32 speed = 60;

				if (rand() % 100 > 18) {
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
				enemy->time_to_live = 0;
			}
		}

		// Clear screen, etc.
		glfwPollEvents();
		glClearColor(0.0, 0.0, 0.0, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(render_context.shader);

		// Render terrain.
		// It's rendered before physics is updated as physics may have some
		// debug rendering.
		render_sprite(TERRAIN_TEXTURE, (vec3){0, 0, 0}, (vec2){256, 224}, 0, (vec4){1, 1, 1, 1}, 0);

		// Update physics.
		physics_tick(context.delta_time, entity_context.entity_array);
		physics_cleanup();

		render_screen_shake(context.delta_time);

		// Render explosion.
		if (context.rocket_explosion_timer > 0) {
			glUseProgram(render_context.circle_shader);
			f32 pct = 1.0f - context.rocket_explosion_timer / EXPLOSION_TIME;
			render_circle(context.rocket_explosion_position[0],
			              context.rocket_explosion_position[1],
			              EXPLOSION_RADIUS * pct, (vec4){1, 1, 1, 1});
			context.rocket_explosion_timer -= context.delta_time;
			rocket_damage(pct);
		}

		glUseProgram(render_context.shader);

		// Spawn rocket smoke.
		if (context.rocket_id > 0) {
			Entity *rocket = &entity_context.entity_array[context.rocket_id];
			if (context.rocket_smoke_timer > 0)
				context.rocket_smoke_timer -= context.delta_time;

			if (rocket->is_in_use && context.rocket_smoke_timer < 0) {
				u32 smoke_id = entity_create(SMOKE_TEXTURE, rocket->aabb.position[0], rocket->aabb.position[1], 0, 0, 10, 10, -5, -5, 5);
				Entity *smoke = &entity_context.entity_array[smoke_id];
				context.rocket_smoke_timer = 0.05f;
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
			Entity *entity = &entity_context.entity_array[i];
			if (entity->is_in_use) {
				if (entity->time_to_live > 0) {
					if (!entity->is_kinematic)
						entity->rotation += context.delta_time * 10;
					entity->time_to_live -= context.delta_time;
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
				render_sprite(entity->texture, position, entity->sprite_size, entity->rotation, entity->sprite_color, entity->is_flipped);

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

		// Render spawn regions.
		// for (u32 i = 0; i < SPAWN_REGION_COUNT; ++i) {
		// 	Spawn_Region spawn_region = BOX_SPAWN_REGIONS[i];
		// 	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		// 	render_quad(spawn_region.x, spawn_region.y, spawn_region.w, spawn_region.h, (vec4){1, 1, 0.5f, 1});
		// 	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		// }
#endif

		// glUseProgram(render_context.text_shader);
		// render_text(130, 130, "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789", (vec4){1, 1, 1, 1});

		glfwSwapBuffers(render_context.window);
	}
}
