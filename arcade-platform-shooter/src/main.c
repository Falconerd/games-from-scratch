#include "shared.h"
#include "render.h"
#include "entity.h"
#include "physics.h"
#include "input.h"

typedef struct game_context {
	f32 time_now;
	f32 time_last_frame;
	f32 delta_time;

	u8 entity_ids[MAX_ENTITIES];
	u8 body_ids[MAX_OBJECTS];

	u32 level_body_count;

	f32 spawn_timer;

	Entity *player;

	u8 jump_key_state;
} Game_Context;

static u32 PLAYER_TEXTURE;
static u32 TERRAIN_TEXTURE;
static u32 ENEMY_TEXTURE;

static Game_Context game_context = {0};
static Physics_Context *physics_context;
static Render_Context *render_context;
static Entity_Context *entity_context;
static Input_Context *input_context;

static const char *GAME_TITLE = "Mega Box Crate";
static const vec2 PLAYER_START_POSITION = {123, 96};
static const vec2 ENEMY_START_POSITION = {123, 220};
static const f32 PLAYER_MOVE_SPEED = 2.0f;
static const f32 PLAYER_JUMP_VELOCITY = 6.0f;
static const u8 PLAYER_COLLISION_MASK = 1;
static const u8 ENEMY_COLLISION_MASK = 2;
static const u8 ENTITY_COLLISION_MASK = 4;
static const f32 ENEMY_VELOCITY = 1.0f;

void restart();

void on_enemy_collide(Body *enemy_body, Body *other_body, DIRECTION direction) {
	if (other_body->is_fixed && (direction == LEFT || direction == RIGHT)) {
		enemy_body->velocity[0] *= -1;
	} else if (other_body->is_fixed && direction == TOP) {
		enemy_body->velocity[1] = 0;
	}

	if (other_body->is_trigger) {
		if (other_body->aabb.min[1] == 0) {
			printf("%f %f\n", enemy_body->aabb.min[0], enemy_body->aabb.min[1]);
		}
	}
}

void spawn_enemies() {
	game_context.spawn_timer -= game_context.delta_time;
	if (game_context.spawn_timer < 0) {
		game_context.spawn_timer = 4;
		Entity *enemy = entity_create(0, (vec2){-2, 0}, (vec2){10, 7}, NULL, 1);
		enemy->texture = ENEMY_TEXTURE;
		enemy->body = physics_create_body((vec2){128, 220}, (vec2){6, 7});
		enemy->body->entity = enemy;
		enemy->body->mask = ENEMY_COLLISION_MASK | ENTITY_COLLISION_MASK;
		enemy->body->velocity[0] = ENEMY_VELOCITY;
		enemy->body->on_collision_enter = on_enemy_collide;
	}
}

void player_movement() {
	f32 horizontal_velocity = 0;
	f32 vertical_velocity = game_context.player->body->velocity[1];

	if ((game_context.player->body->collision_direction & TOP) != 0) {
		vertical_velocity = 0;
	}

	if (glfwGetKey(render_context->window, GLFW_KEY_F) == GLFW_RELEASE && game_context.jump_key_state == GLFW_PRESS) {
		vertical_velocity *= 0.5f;
		game_context.jump_key_state = GLFW_RELEASE;
	}

	if (glfwGetKey(render_context->window, GLFW_KEY_R) == GLFW_PRESS) {
		horizontal_velocity += -PLAYER_MOVE_SPEED;
		game_context.player->flipped = 1;
	}

	if (glfwGetKey(render_context->window, GLFW_KEY_T) == GLFW_PRESS) {
		horizontal_velocity += PLAYER_MOVE_SPEED;
		game_context.player->flipped = 0;
	}

	if (glfwGetKey(render_context->window, GLFW_KEY_F) == GLFW_PRESS) {
		game_context.jump_key_state = GLFW_PRESS;
		if ((game_context.player->body->collision_direction & TOP) != 0) {
			vertical_velocity = PLAYER_JUMP_VELOCITY;
		}
	}

	if ((game_context.player->body->collision_direction & BOTTOM) != 0) {
		vertical_velocity = 0;
	}

	game_context.player->body->velocity[0] = horizontal_velocity;
	game_context.player->body->velocity[1] = vertical_velocity;
}

void on_player_collide(Body *player_body, Body *other_body, DIRECTION direction) {
	if (other_body->is_fixed == 0) {
		restart();
	}
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	render_square(player_body->aabb.min[0], player_body->aabb.min[1], player_body->aabb.max[0] - player_body->aabb.min[0], player_body->aabb.max[1] - player_body->aabb.min[1], (vec4){1, 0, 0, 1});
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void restart() {
	physics_reset();
	entity_reset();

	game_context.player = entity_create(0, (vec2){-12, 0}, (vec2){32, 10}, NULL, 1);
	Entity *player = game_context.player;
	// setup player
	player->texture = PLAYER_TEXTURE;
	player->body = physics_create_body((vec2){PLAYER_START_POSITION[0], PLAYER_START_POSITION[1]}, (vec2){8, 10});
	player->body->mask = PLAYER_COLLISION_MASK | ENTITY_COLLISION_MASK;
	player->body->on_collision_enter = on_player_collide;
	player->body->entity = player;

	{ // create terrain bodies
		Body *fixed;
		fixed = physics_create_body((vec2){0, 0}, (vec2){112, 32});
		fixed->is_fixed = 1;
		fixed->mask = PLAYER_COLLISION_MASK | ENEMY_COLLISION_MASK;
		fixed = physics_create_body((vec2){144, 0}, (vec2){112, 32});
		fixed->is_fixed = 1;
		fixed->mask = PLAYER_COLLISION_MASK | ENEMY_COLLISION_MASK;
		fixed = physics_create_body((vec2){112, 0}, (vec2){32, 32});
		fixed->is_fixed = 1;
		fixed->mask = PLAYER_COLLISION_MASK;
		fixed = physics_create_body((vec2){64, 64}, (vec2){128, 13});
		fixed->is_fixed = 1;
		fixed->mask = PLAYER_COLLISION_MASK | ENEMY_COLLISION_MASK;
		fixed = physics_create_body((vec2){64, 160}, (vec2){128, 13});
		fixed->is_fixed = 1;
		fixed->mask = PLAYER_COLLISION_MASK | ENEMY_COLLISION_MASK;
		fixed = physics_create_body((vec2){13, 112}, (vec2){51, 13});
		fixed->is_fixed = 1;
		fixed->mask = PLAYER_COLLISION_MASK | ENEMY_COLLISION_MASK;
		fixed = physics_create_body((vec2){192, 112}, (vec2){51, 13});
		fixed->is_fixed = 1;
		fixed->mask = PLAYER_COLLISION_MASK | ENEMY_COLLISION_MASK;
		fixed = physics_create_body((vec2){0, 211}, (vec2){112, 13});
		fixed->is_fixed = 1;
		fixed->mask = PLAYER_COLLISION_MASK | ENEMY_COLLISION_MASK;
		fixed = physics_create_body((vec2){144, 211}, (vec2){112, 13});
		fixed->is_fixed = 1;
		fixed->mask = PLAYER_COLLISION_MASK | ENEMY_COLLISION_MASK;
		fixed = physics_create_body((vec2){112, 211}, (vec2){32, 13});
		fixed->is_fixed = 1;
		fixed->mask = PLAYER_COLLISION_MASK;
		fixed = physics_create_body((vec2){0, 32}, (vec2){13, 179});
		fixed->is_fixed = 1;
		fixed->mask = PLAYER_COLLISION_MASK | ENEMY_COLLISION_MASK;
		fixed = physics_create_body((vec2){243, 32}, (vec2){13, 179});
		fixed->is_fixed = 1;
		fixed->mask = PLAYER_COLLISION_MASK | ENEMY_COLLISION_MASK;

		fixed = physics_create_body((vec2){112, 0}, (vec2){32, 16});
		fixed->is_fixed = 1;
		fixed->is_trigger = 1;
		fixed->mask = ENEMY_COLLISION_MASK;

		game_context.level_body_count = physics_context->body_count;
	}
}

int main(void) {
	srand(time(NULL));

	physics_context = physics_setup();
	render_context = render_setup(GAME_TITLE);
	entity_context = entity_setup();
	input_context = input_setup();

	PLAYER_TEXTURE = render_create_texture("./assets/player.png");
	ENEMY_TEXTURE = render_create_texture("./assets/enemy.png");
	TERRAIN_TEXTURE = render_create_texture("./assets/terrain.png");

	restart();

	while (!glfwWindowShouldClose(render_context->window)) {
		game_context.time_now = glfwGetTime();
		game_context.delta_time = game_context.time_now - game_context.time_last_frame;
		game_context.time_last_frame = game_context.time_now;

		glfwPollEvents();
		glClearColor(0.9, 0.8, 0.8, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		spawn_enemies();
		player_movement();

		physics_tick(game_context.delta_time);

		glUseProgram(render_context->shader);
		glUniformMatrix4fv(glGetUniformLocation(render_context->shader, "projection"), 1, GL_FALSE, &render_context->projection[0][0]);

		render_sprite(TERRAIN_TEXTURE, (vec3){0, 0, 0}, (vec2){WIDTH, HEIGHT}, 0);

		for (u32 i = 0; i < physics_context->body_count; ++i) {
			AABB *aabb = &physics_context->bodies[i].aabb;
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			render_square(aabb->min[0], aabb->min[1], aabb->max[0] - aabb->min[0], aabb->max[1] - aabb->min[1], (vec4){0.0f, 1.0f, 0.0f, 1.0f});
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		for (u32 i = 0; i < entity_context->entity_count; ++i) {
			Entity *entity = &entity_context->entities[i];
			if (entity->texture != 0xdeadbeef) {
				vec3 position = { entity->body->aabb.min[0] + entity->offset[0],
						  entity->body->aabb.min[1] + entity->offset[1], 0 };
				render_sprite(entity->texture, position, entity->size, entity->flipped);
			}
		}

		glfwSwapBuffers(render_context->window);
	}
}
