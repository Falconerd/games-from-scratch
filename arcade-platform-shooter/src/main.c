#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shared.h"
#include "render.h"
#include "entity.h"
#include "physics.h"
#include "input.h"

static f32 time_now;
static f32 time_last_frame;
static f32 delta_time;
static f32 player_jump_start_y;
static u8 player_can_extend_jump = 1;

static const f32 PLAYER_MOVE_SPEED = 2.0f;
static const f32 PLAYER_JUMP_VELOCITY = 4.5f;
static const f32 PLAYER_JUMP_CUTOFF = 20.0f;

static Physics_Context *physics_context;
static Render_Context *render_context;
static Entity_Context *entity_context;
static Input_Context *input_context;

void player_movement(GLFWwindow *window, Entity *player, Input_Context *input_context) {
	f32 horizontal_velocity = 0;
	f32 vertical_velocity = player->body->velocity[1];

	if (player->body->aabb.min[1] > player_jump_start_y + PLAYER_JUMP_CUTOFF) {
			player_can_extend_jump = 0;
	}

	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
		horizontal_velocity += -PLAYER_MOVE_SPEED;
	}
	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
		horizontal_velocity += PLAYER_MOVE_SPEED;
	}
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
		if (player->body->collision_direction == TOP) {
			vertical_velocity = PLAYER_JUMP_VELOCITY;
			player_jump_start_y = player->body->aabb.min[1];
			player_can_extend_jump = 1;
		} else if (player->body->aabb.min[1] < player_jump_start_y + PLAYER_JUMP_CUTOFF && player_can_extend_jump) {
			vertical_velocity = PLAYER_JUMP_VELOCITY;
		}
	}

	player->body->velocity[0] = horizontal_velocity;
	player->body->velocity[1] = vertical_velocity;
}

void on_player_collide(Body *player_body, Body *other_body, DIRECTION direction) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	render_square(player_body->aabb.min[0], player_body->aabb.min[1], player_body->aabb.max[0] - player_body->aabb.min[0], player_body->aabb.max[1] - player_body->aabb.min[1], (vec4){1, 0, 0, 1});
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

int main(void) {
	physics_context = physics_setup();
	render_context = render_setup();
	entity_context = entity_setup();
	input_context = input_setup();

	Entity *player = entity_create(0, (vec2){-2, 0}, (vec2){14, 10}, NULL, 1);
	{ // setup player
		player->texture = render_create_texture("./assets/player.png");
		player->body = physics_create_body((vec2){100, 100}, (vec2){8, 8});
		player->body->on_collision_enter = on_player_collide;
		player->body->entity_id = entity_context->entity_count - 1;
	}

	{ // create terrain bodies
		Body *fixed = physics_create_body((vec2){0, 0}, (vec2){WIDTH, 20});
		fixed->fixed = 1;
		// fixed = physics_create_body((vec2){WIDTH - WIDTH/2.5f, 22}, (vec2){WIDTH/2.5f, 20});
		// fixed->fixed = 1;
	}

	while (!glfwWindowShouldClose(render_context->window)) {
		time_now = glfwGetTime();
		delta_time = time_now - time_last_frame;
		time_last_frame = time_now;

		glfwPollEvents();
		glClearColor(0.1, 0.4, 0.5, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		player_movement(render_context->window, player, input_context);

		physics_tick(delta_time);

		glUseProgram(render_context->shader);
		glUniformMatrix4fv(glGetUniformLocation(render_context->shader, "projection"), 1, GL_FALSE, &render_context->projection[0][0]);

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
				render_sprite(entity->texture, position, entity->size);
			}
		}

		glfwSwapBuffers(render_context->window);
	}
}
