#include "shared.h"

typedef struct game_context {
	f32 time_now;
	f32 time_last_frame;
	f32 delta_time;

	u8 jump_key_state;
	f32 spawn_timer;
	f32 box_spawn_timer;
	Entity *player;
} Game_Context;

static u32 PLAYER_TEXTURE;
static u32 PLAYER_TEXTURE_MACHINE_GUN;
static u32 PLAYER_TEXTURE_SHOTGUN;
static u32 PLAYER_TEXTURE_BAZOOKA;
static u32 TERRAIN_TEXTURE;
static u32 ENEMY_TEXTURE_SMALL;
static u32 ENEMY_TEXTURE_LARGE;
static u32 ENEMY_TEXTURE_SMALL_ANGRY;
static u32 ENEMY_TEXTURE_LARGE_ANGRY;
static u32 BOX_TEXTURE;
static u32 BULLET_TEXTURE;

static Game_Context context = {0};
static Render_Context *render_context;
static Entity_Context *entity_context;
static Input_Context *input_context;

static const char *GAME_TITLE = "Mega Box Crate";
static const f32 PLAYER_MOVE_SPEED = 2.0f;
static const f32 PLAYER_JUMP_VELOCITY = 5.0f;
static const u8 PLAYER_COLLISION_MASK = 1;
static const u8 ENEMY_COLLISION_MASK = 2;
static const u8 ENTITY_COLLISION_MASK = 4;
static const u8 BOX_MASK = 8;

typedef enum enemy_type {
	ENEMY_TYPE_SMALL,
	ENEMY_TYPE_LARGE,
	ENEMY_TYPE_SMALL_ANGRY,
	ENEMY_TYPE_LARGE_ANGRY
} Enemy_Type;

typedef struct enemy_template {
	vec2 aabb_size;   
	vec2 aabb_offset;
	vec2 sprite_size;
} Enemy_Template;

Enemy_Template enemy_template_small = { {6, 7}, {-2, 0}, {10, 7} };
Enemy_Template enemy_template_large = { {14, 12}, {-2, 0}, {16, 14} };

typedef enum gun_type {
	GUN_TYPE_MACHINE_GUN,
	GUN_TYPE_SHOTGUN,
	GUN_TYPE_BAZOOKA,
	GUN_TYPE_COUNT
} Gun_Type;

void restart();
void spawn_enemy(Enemy_Type type);

void on_enemy_collide(Body *enemy_body, Body *other_body, DIRECTION direction) {
	Entity *enemy = (Entity*)enemy_body;
	Entity *other = (Entity*)other_body;
	if ((other->body.flags & BODY_IS_FIXED) != 0 && (direction == LEFT || direction == RIGHT)) {
		enemy->body.velocity[0] *= -1;
	} else if ((other->body.flags & BODY_IS_FIXED) != 0 && direction == TOP) {
		enemy->body.velocity[1] = 0;
	}

	if (enemy->body.velocity[0] > 0) {
		enemy->flags &= ~ENTITY_IS_FLIPPED;
	} else {
		enemy->flags |= ENTITY_IS_FLIPPED;
	}

	if ((other->body.flags & BODY_IS_TRIGGER) != 0) {
		if (other->body.aabb.min[1] == 0) {
			if (enemy->texture == ENEMY_TEXTURE_SMALL || enemy->texture == ENEMY_TEXTURE_SMALL_ANGRY) {
				spawn_enemy(ENEMY_TYPE_SMALL_ANGRY);
			} else if (enemy->texture == ENEMY_TEXTURE_LARGE || enemy->texture == ENEMY_TEXTURE_LARGE_ANGRY) {
				spawn_enemy(ENEMY_TYPE_LARGE_ANGRY);
			}
			entity_destroy(enemy);
		}
	}
}

void spawn_enemy(Enemy_Type type) {
	u32 texture;
	Enemy_Template *enemy_template;
	u8 is_flipped = rand() % 100 > 50;
	f32 velocity = 1;
	u8 health;

	switch (type) {
		case ENEMY_TYPE_SMALL: {
			texture = ENEMY_TEXTURE_SMALL;
			enemy_template = &enemy_template_small;
			health = 1;
		} break;
		case ENEMY_TYPE_LARGE: {
			texture = ENEMY_TEXTURE_LARGE;
			enemy_template = &enemy_template_large;
			health = 3;
		} break;
		case ENEMY_TYPE_SMALL_ANGRY: {
			texture = ENEMY_TEXTURE_SMALL_ANGRY;
			enemy_template = &enemy_template_small;
			velocity = 2;
			health = 2;
		} break;
		case ENEMY_TYPE_LARGE_ANGRY: {
			texture = ENEMY_TEXTURE_LARGE_ANGRY;
			enemy_template = &enemy_template_large;
			velocity = 1.5f;
			health = 6;
		} break;
	}

	Entity *enemy = entity_create(texture, (vec2){123, 220}, enemy_template->aabb_size, enemy_template->aabb_offset, enemy_template->sprite_size, 1);

	enemy->health = health;

	if (is_flipped) {
		enemy->flags |= ENTITY_IS_FLIPPED;
		enemy->body.velocity[0] = -velocity;
	} else {
		enemy->body.velocity[0] = -velocity;
	}
	enemy->flags |= ENTITY_IS_ENEMY;
	enemy->body.mask = ENEMY_COLLISION_MASK | ENTITY_COLLISION_MASK;
	enemy->body.on_collision = on_enemy_collide;
}

void spawn_enemies() {
	context.spawn_timer -= context.delta_time;
	if (context.spawn_timer < 0) {
		context.spawn_timer = (rand() % 5) + 1;
		spawn_enemy(rand() % 100 > 25 ? ENEMY_TYPE_SMALL : ENEMY_TYPE_LARGE);
	}
}

void spawn_box() {
	context.box_spawn_timer -= context.delta_time;
	if (context.box_spawn_timer < 0) {
		context.box_spawn_timer = (rand() % 10) + 5;

		// Get box spawning location.
		vec2 position = {32, 32};
		Entity *box = entity_create(BOX_TEXTURE, position, (vec2){8, 8}, (vec2){0, 0}, (vec2){8, 8}, 1);
		box->body.flags |= BODY_IS_TRIGGER | BODY_IS_FIXED;
		box->body.mask = BOX_MASK;
	}
}

void on_bullet_collide(Body *self_body, Body *other_body, DIRECTION direction) {
	Entity *self = (Entity*)self_body;
	Entity *other = (Entity*)other_body;

	if ((other->flags & ENTITY_IS_ENEMY) != 0) {
		entity_destroy(self);
		other->health--;
		if (other->health <= 0) {
			entity_destroy(other);
		}
	}

	if ((other->body.flags & BODY_IS_FIXED) != 0) {
		entity_destroy(self);
	}
}

void player_input() {
	f32 horizontal_velocity = 0;
	f32 vertical_velocity = context.player->body.velocity[1];

	if ((context.player->body.flags & TOP) != 0) {
		vertical_velocity = 0;
	}

	if (glfwGetKey(render_context->window, GLFW_KEY_F) == GLFW_RELEASE && context.jump_key_state == GLFW_PRESS) {
		vertical_velocity *= 0.5f;
		context.jump_key_state = GLFW_RELEASE;
	}

	if (glfwGetKey(render_context->window, GLFW_KEY_R) == GLFW_PRESS) {
		horizontal_velocity += -PLAYER_MOVE_SPEED;
		context.player->flags |= ENTITY_IS_FLIPPED;
	}

	if (glfwGetKey(render_context->window, GLFW_KEY_T) == GLFW_PRESS) {
		horizontal_velocity += PLAYER_MOVE_SPEED;
		context.player->flags &= ~ENTITY_IS_FLIPPED;
	}

	if (glfwGetKey(render_context->window, GLFW_KEY_F) == GLFW_PRESS) {
		context.jump_key_state = GLFW_PRESS;
		if ((context.player->body.flags & TOP) != 0) {
			vertical_velocity = PLAYER_JUMP_VELOCITY;
		}
	}

	if (glfwGetKey(render_context->window, GLFW_KEY_E) == GLFW_PRESS) {
		if (context.player->texture == PLAYER_TEXTURE_MACHINE_GUN) {
			vec3 start_position = {context.player->body.aabb.min[0] + 4, context.player->body.aabb.min[1] + 2};
			Entity *bullet = entity_create(BULLET_TEXTURE, start_position, (vec2){3, 3}, (vec2){0, 0}, (vec2){3, 3}, 0);
			bullet->body.velocity[0] = 5;
			if ((context.player->flags & ENTITY_IS_FLIPPED) != 0) {
				bullet->body.velocity[0] = -5;
			}
			bullet->body.flags |= BODY_IS_KINEMATIC;
			bullet->body.on_collision = on_bullet_collide;
		}
	}

	if ((context.player->body.flags & BOTTOM) != 0) {
		vertical_velocity = 0;
	}

	context.player->body.velocity[0] = horizontal_velocity;
	context.player->body.velocity[1] = vertical_velocity;
}

void on_player_collide(Body *player_body, Body *other_body, DIRECTION direction) {
	if ((other_body->flags & BODY_IS_FIXED) == 0) {
		// restart();
	}

	// Pick up gun.
	if ((other_body->mask & BOX_MASK) != 0) {
		u32 type = rand() % GUN_TYPE_COUNT;
		switch (type) {
		case GUN_TYPE_MACHINE_GUN: {
			context.player->texture = PLAYER_TEXTURE_MACHINE_GUN;
			entity_destroy((Entity*)other_body);
		} break;
		case GUN_TYPE_SHOTGUN: {
			context.player->texture = PLAYER_TEXTURE_SHOTGUN;
		} break;
		case GUN_TYPE_BAZOOKA: {
			context.player->texture = PLAYER_TEXTURE_BAZOOKA;
		} break;
		}
	}
}

Entity *create_terrain(vec2 position, vec2 size, u8 mask) {
	Entity *terrain = entity_create(0, position, size, (vec2){0, 0}, (vec2){0, 0}, 0);
	terrain->body.flags |= BODY_IS_FIXED;
	terrain->body.mask = mask;
	return terrain;
}

void restart() {
	entity_reset();
	physics_reset();

	// Setup player.
	context.player = entity_create(PLAYER_TEXTURE, (vec2){123, 96}, (vec2){8, 10}, (vec2){-12, 0}, (vec2){32, 10}, 1);
	context.player->body.mask = PLAYER_COLLISION_MASK | ENTITY_COLLISION_MASK | BOX_MASK;
	context.player->body.on_collision = on_player_collide;

	// Create terrain.
	create_terrain((vec2){0, 0}, (vec2){112, 32}, PLAYER_COLLISION_MASK | ENEMY_COLLISION_MASK);
	create_terrain((vec2){144, 0}, (vec2){112, 32}, PLAYER_COLLISION_MASK | ENEMY_COLLISION_MASK);
	create_terrain((vec2){64, 64}, (vec2){128, 13}, PLAYER_COLLISION_MASK | ENEMY_COLLISION_MASK);
	create_terrain((vec2){64, 160}, (vec2){128, 13}, PLAYER_COLLISION_MASK | ENEMY_COLLISION_MASK);
	create_terrain((vec2){13, 112}, (vec2){51, 13}, PLAYER_COLLISION_MASK | ENEMY_COLLISION_MASK);
	create_terrain((vec2){192, 112}, (vec2){51, 13}, PLAYER_COLLISION_MASK | ENEMY_COLLISION_MASK);
	create_terrain((vec2){0, 211}, (vec2){112, 13}, PLAYER_COLLISION_MASK | ENEMY_COLLISION_MASK);
	create_terrain((vec2){144, 211}, (vec2){112, 13}, PLAYER_COLLISION_MASK | ENEMY_COLLISION_MASK);
	create_terrain((vec2){0, 32}, (vec2){13, 179}, PLAYER_COLLISION_MASK | ENEMY_COLLISION_MASK);
	create_terrain((vec2){243, 32}, (vec2){13, 179}, PLAYER_COLLISION_MASK | ENEMY_COLLISION_MASK);

	create_terrain((vec2){112, 0}, (vec2){32, 32}, PLAYER_COLLISION_MASK);
	create_terrain((vec2){112, 211}, (vec2){32, 13}, PLAYER_COLLISION_MASK);

	Entity *e = create_terrain((vec2){112, 0}, (vec2){32, 16}, ENEMY_COLLISION_MASK);
	e->body.flags |= BODY_IS_TRIGGER;
}

int main(void) {
	srand(time(NULL));

	render_context = render_setup(GAME_TITLE);
	entity_context = entity_setup();
	physics_setup(entity_context);
	input_context = input_setup();

	PLAYER_TEXTURE = render_create_texture("./assets/player.png");
	PLAYER_TEXTURE_MACHINE_GUN = render_create_texture("./assets/player_machine_gun.png");
	PLAYER_TEXTURE_SHOTGUN = render_create_texture("./assets/player_shotgun.png");
	PLAYER_TEXTURE_BAZOOKA = render_create_texture("./assets/player_bazooka.png");
	ENEMY_TEXTURE_SMALL = render_create_texture("./assets/enemy_small.png");
	ENEMY_TEXTURE_LARGE = render_create_texture("./assets/enemy_large.png");
	ENEMY_TEXTURE_SMALL_ANGRY = render_create_texture("./assets/enemy_small_angry.png");
	ENEMY_TEXTURE_LARGE_ANGRY = render_create_texture("./assets/enemy_large_angry.png");
	TERRAIN_TEXTURE = render_create_texture("./assets/terrain.png");
	BOX_TEXTURE = render_create_texture("./assets/box.png");
	BULLET_TEXTURE = render_create_texture("./assets/bullet.png");

	restart();

	while (!glfwWindowShouldClose(render_context->window)) {
		context.time_now = glfwGetTime();
		context.delta_time = context.time_now - context.time_last_frame;
		context.time_last_frame = context.time_now;

		glfwPollEvents();
		glClearColor(0.9, 0.8, 0.8, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		spawn_enemies();
		spawn_box();
		player_input();

		physics_tick(context.delta_time);

		glUseProgram(render_context->shader);
		glUniformMatrix4fv(glGetUniformLocation(render_context->shader, "projection"), 1, GL_FALSE, &render_context->projection[0][0]);

		render_sprite(TERRAIN_TEXTURE, (vec3){0, 0, 0}, (vec2){WIDTH, HEIGHT}, 0);

		for (u32 i = 0; i < MAX_ENTITIES; ++i) {
			Entity *entity = &entity_context->entities[i];
			if ((entity->flags & ENTITY_IS_IN_USE) == 0) {
				continue;
			}
			AABB *aabb = &entity_context->entities[i].body.aabb;
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			render_square(aabb->min[0], aabb->min[1], aabb->max[0] - aabb->min[0], aabb->max[1] - aabb->min[1], (vec4){0.0f, 1.0f, 0.0f, 1.0f});
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		for (u32 i = 0; i < MAX_ENTITIES; ++i) {
			Entity *entity = &entity_context->entities[i];
			if ((entity->flags & ENTITY_IS_IN_USE) == 0) {
				continue;
			}
			if (entity->texture != 0xdeadbeef) {
				vec3 position = { entity->body.aabb.min[0] + entity->sprite_offset[0],
						  entity->body.aabb.min[1] + entity->sprite_offset[1], 0 };
				render_sprite(entity->texture, position, entity->sprite_size, (entity->flags & ENTITY_IS_FLIPPED) != 0);
			}
		}

		glfwSwapBuffers(render_context->window);
	}
}
