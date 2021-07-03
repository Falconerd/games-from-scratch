#include "shared.h"
#include "render.h"
#include "entity.h"
#include "physics.h"
#include "input.h"

typedef struct game_context {
	f32 time_now;
	f32 time_last_frame;
	f32 delta_time;

	u8 jump_key_state;
	f32 spawn_timer;
	Entity *player;
} Game_Context;

static u32 PLAYER_TEXTURE;
static u32 TERRAIN_TEXTURE;
static u32 ENEMY_TEXTURE;

static Game_Context context = {0};
static Render_Context *render_context;
static Entity_Context *entity_context;
static Input_Context *input_context;

static const char *GAME_TITLE = "Mega Box Crate";
static const vec2 PLAYER_START_POSITION = {123, 96};
// static const vec2 ENEMY_START_POSITION = {123, 220};
static const f32 PLAYER_MOVE_SPEED = 2.0f;
static const f32 PLAYER_JUMP_VELOCITY = 6.0f;
static const f32 ENEMY_VELOCITY = 1.0f;
static const u8 PLAYER_COLLISION_MASK = 1;
static const u8 ENEMY_COLLISION_MASK = 2;
static const u8 ENTITY_COLLISION_MASK = 4;

typedef enum enemy_type {
	ENEMY_TYPE_SMALL,
	ENEMY_TYPE_LARGE,
	ENEMY_TYPE_SMALL_ANGRY,
	ENEMY_TYPE_LARGE_ANGRY
} Enemy_Type;

void restart();

void on_enemy_collide(Body *enemy_body, Body *other_body, DIRECTION direction) {
	Entity *enemy = (Entity*)enemy_body;
	Entity *other = (Entity*)other_body;
	if ((other->body.flags & BODY_IS_FIXED) != 0 && (direction == LEFT || direction == RIGHT)) {
		enemy->body.velocity[0] *= -1;
	} else if ((other->body.flags & BODY_IS_FIXED) != 0 && direction == TOP) {
		enemy->body.velocity[1] = 0;
	}

	if ((other->body.flags & BODY_IS_TRIGGER) != 0) {
		if (other->body.aabb.min[1] == 0) {
			// printf("%f %f\n", enemy->body.aabb.min[0], enemy->body.aabb.min[1]);
			entity_destroy(enemy);
		}
	}
}

void spawn_enemy(Enemy_Type type) {
	Entity *enemy = entity_create(ENEMY_TEXTURE, (vec2){128, 220}, (vec2){6, 7}, (vec2){-2, 0}, (vec2){10, 7}, 1);

	enemy->body.mask = ENEMY_COLLISION_MASK | ENTITY_COLLISION_MASK;
	enemy->body.velocity[0] = (rand() > 0.5) ? ENEMY_VELOCITY : -ENEMY_VELOCITY;
	enemy->body.on_collision = on_enemy_collide;
}

void spawn_enemies() {
	context.spawn_timer -= context.delta_time;
	if (context.spawn_timer < 0) {
		context.spawn_timer = 0.2f;
		spawn_enemy(ENEMY_TYPE_SMALL);
	}
}

void player_movement() {
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

	if ((context.player->body.flags & BOTTOM) != 0) {
		vertical_velocity = 0;
	}

	context.player->body.velocity[0] = horizontal_velocity;
	context.player->body.velocity[1] = vertical_velocity;
}

void on_player_collide(Body *player_body, Body *other_body, DIRECTION direction) {
	if ((other_body->flags & BODY_IS_FIXED) == 0) {
		restart();
	}
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	render_square(player_body->aabb.min[0], player_body->aabb.min[1], player_body->aabb.max[0] - player_body->aabb.min[0], player_body->aabb.max[1] - player_body->aabb.min[1], (vec4){1, 0, 0, 1});
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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
	context.player = entity_create(PLAYER_TEXTURE, (vec2){PLAYER_START_POSITION[0], PLAYER_START_POSITION[1]}, (vec2){8, 10}, (vec2){-12, 0}, (vec2){32, 10}, 1);
	context.player->body.mask = PLAYER_COLLISION_MASK | ENTITY_COLLISION_MASK;
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
	ENEMY_TEXTURE = render_create_texture("./assets/enemy.png");
	TERRAIN_TEXTURE = render_create_texture("./assets/terrain.png");

	restart();

	while (!glfwWindowShouldClose(render_context->window)) {
		context.time_now = glfwGetTime();
		context.delta_time = context.time_now - context.time_last_frame;
		context.time_last_frame = context.time_now;

		glfwPollEvents();
		glClearColor(0.9, 0.8, 0.8, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		spawn_enemies();
		player_movement();

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
