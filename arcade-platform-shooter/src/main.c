#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shared.h"
#include "render.h"
#include "entity.h"
#include "physics.h"

static f32 time_now;
static f32 time_last_frame;
static f32 delta_time;

int main(void) {
	Physics_Context *physics_context = physics_setup();
	Render_Context *render_context = render_setup();
	Entity_Context *entity_context = entity_setup();

	Entity *player = entity_create();
	Entity *enemy1 = entity_create();

	player->texture = render_create_texture("./assets/player.png");
	player->position[0] = 100;
	player->position[1] = 100;
	player->size[0] = 10;
	player->size[1] = 10;

	enemy1->texture = render_create_texture("./assets/enemy_small.png");
	enemy1->position[0] = 40;
	enemy1->position[1] = 40;
	enemy1->size[0] = 7;
	enemy1->size[1] = 7;

	{
		u32 fixed = physics_create_body((vec2){100, 40}, (vec2){WIDTH/4, 50});
		physics_context->bodies[fixed].fixed = 1;
		u32 x;

		x = physics_create_body((vec2){10, 50}, (vec2){10, 10});
		physics_context->bodies[x].velocity[0] = 1;
		physics_context->bodies[x].velocity[1] = 0.2f;

		x = physics_create_body((vec2){10, 40}, (vec2){10, 10});
		physics_context->bodies[x].velocity[0] = 1;
		physics_context->bodies[x].velocity[1] = 0.15f;

		x = physics_create_body((vec2){10, 110}, (vec2){10, 15});
		physics_context->bodies[x].velocity[0] = 0.5f;
		physics_context->bodies[x].velocity[1] = -0.07f;

		x = physics_create_body((vec2){220, 110}, (vec2){10, 15});
		physics_context->bodies[x].velocity[0] = -0.5f;
		physics_context->bodies[x].velocity[1] = -0.125f;

		x = physics_create_body((vec2){66, 200}, (vec2){10, 10});
		physics_context->bodies[x].velocity[0] = 0.65f;
		physics_context->bodies[x].velocity[1] = -4;

		x = physics_create_body((vec2){100, 200}, (vec2){10, 10});
		physics_context->bodies[x].velocity[0] = 0;
		physics_context->bodies[x].velocity[1] = -1.5f;

		x = physics_create_body((vec2){200, 40}, (vec2){10, 10});
		physics_context->bodies[x].velocity[0] = -1;
		physics_context->bodies[x].velocity[1] = 0.15f;

		x = physics_create_body((vec2){200, 50}, (vec2){10, 10});
		physics_context->bodies[x].velocity[0] = -1;
		physics_context->bodies[x].velocity[1] = 0.2f;

		x = physics_create_body((vec2){90, 0}, (vec2){10, 10});
		physics_context->bodies[x].velocity[0] = 0.15f;
		physics_context->bodies[x].velocity[1] = 1;

		x = physics_create_body((vec2){140, 0}, (vec2){10, 10});
		physics_context->bodies[x].velocity[0] = 0.25f;
		physics_context->bodies[x].velocity[1] = 1;

		x = physics_create_body((vec2){200, 200}, (vec2){10, 10});
		physics_context->bodies[x].velocity[0] = -0.25f;
		physics_context->bodies[x].velocity[1] = -1;

		x = physics_create_body((vec2){130, 200}, (vec2){10, 10});
		physics_context->bodies[x].velocity[0] = 0.25f;
		physics_context->bodies[x].velocity[1] = -1;
	}


	while(!glfwWindowShouldClose(render_context->window)) {
		time_now = glfwGetTime();
		delta_time = time_now - time_last_frame;
		time_last_frame = time_now;

		glfwPollEvents();
		glClearColor(0.1, 0.4, 0.5, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(render_context->shader);
		glUniformMatrix4fv(glGetUniformLocation(render_context->shader, "projection"), 1, GL_FALSE, &render_context->projection[0][0]);

		physics_tick(delta_time);

		for (u32 i = 0; i < physics_context->body_count; ++i) {
			AABB *aabb = &physics_context->bodies[i].aabb;
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			render_square(aabb->min[0], aabb->min[1], aabb->max[0] - aabb->min[0], aabb->max[1] - aabb->min[1], (vec4){0.0f, 1.0f, 0.0f, 1.0f});
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		for (u32 i = 0; i < entity_context->entity_count; ++i) {
			Entity *entity = &entity_context->entities[i];
			if (entity->texture != 0xdeadbeef) {
				render_sprite(entity->texture, entity->position, entity->size);
			}
		}

		glfwSwapBuffers(render_context->window);
	}
}
