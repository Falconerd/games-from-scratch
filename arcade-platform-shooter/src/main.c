#include "shared.h"
#define PI 3.1415f

typedef struct game_context {
	f32 time_now;
	f32 time_last_frame;
	f32 delta_time;

	u32 jump_key_state;
	f32 spawn_timer;
	f32 box_spawn_timer;
} Game_Context;

static Game_Context context = {0};

static const char *GAME_TITLE = "Mega Box Crate";
static const Render_Context *render_context;
static const Physics_Context *physics_context;
static const Entity_Context *entity_context;

static u32 TEXTURE_TERRAIN;
static u32 TEXTURE_PLAYER;

int main(void) {
	srand(time(NULL));

	physics_context = physics_setup(256, 10);
	render_context = render_setup(GAME_TITLE);
	entity_context = entity_setup(256);

	// Setup textures.
	{
		TEXTURE_TERRAIN = render_texture_create("./assets/terrain.png");
		TEXTURE_PLAYER = render_texture_create("./assets/player.png");
	}

	// Setup player.
	Entity *entity_player = entity_create(TEXTURE_PLAYER, (vec2){0, 0}, (vec2){32, 10});
	Body *body_player = physics_body_create((vec2){48, 38}, (vec2){4, 4});
	body_player->velocity[0] = 2;

	// Setup terrain.
	{
		// Bottom.
		physics_static_body_create((vec2){128, 16}, (vec2){256, 16});
		// physics_static_body_create((vec2){200, 16}, (vec2){56, 16});
		// Top.
		physics_static_body_create((vec2){56, HEIGHT - 6.5f}, (vec2){56, 6.5f});
		physics_static_body_create((vec2){200, HEIGHT - 6.5f}, (vec2){56, 6.5f});
		// Left.
		physics_static_body_create((vec2){6.5f, HEIGHT / 2 + 9.5f}, (vec2){6.5f, HEIGHT / 2 - 6.5f - 16});
		// Right.
		physics_static_body_create((vec2){WIDTH - 6.5f, HEIGHT / 2 + 9.5f}, (vec2){6.5f, HEIGHT / 2 - 6.5f - 16});
	}

	while (!glfwWindowShouldClose(render_context->window)) {
		context.time_now = glfwGetTime();
		context.delta_time = context.time_now - context.time_last_frame;
		context.time_last_frame = context.time_now;

		// Input.
		{
			body_player->velocity[0] = 0;
			body_player->velocity[1] = 0;
			if (glfwGetKey(render_context->window, GLFW_KEY_T) == GLFW_PRESS) {
				body_player->velocity[0] += 1;
			}
			if (glfwGetKey(render_context->window, GLFW_KEY_R) == GLFW_PRESS) {
				body_player->velocity[0] -= 1;
			}
			if (glfwGetKey(render_context->window, GLFW_KEY_F) == GLFW_PRESS) {
				body_player->velocity[1] += 1;
			}
			if (glfwGetKey(render_context->window, GLFW_KEY_S) == GLFW_PRESS) {
				body_player->velocity[1] -= 1;
			}
		}

		glfwPollEvents();
		glClearColor(0.0, 0.0, 0.0, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(render_context->shader);
		glUniformMatrix4fv(glGetUniformLocation(render_context->shader, "projection"), 1, GL_FALSE, &render_context->projection[0][0]);

		// Render terrain.
		// render_sprite(TEXTURE_TERRAIN, (vec3){0, 0, 0}, (vec2){256, 224}, 0);

		// Update physics.

		physics_tick(context.delta_time);

		for (u32 i = 0; i < physics_context->body_array_count; ++i) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			render_aabb(physics_context->body_array[i].aabb, (vec4){0, 1, 0, 1});
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		for (u32 i = 0; i < physics_context->static_body_array_count; ++i) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			render_aabb(physics_context->static_body_array[i].aabb, (vec4){1, 1, 1, 1});
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}


		// Render player.
		{
			vec3 position = {body_player->aabb.position[0] - entity_player->sprite_size[0] * 0.5f, body_player->aabb.position[1] - entity_player->sprite_size[1] * 0.5f + 1, 0};
			// render_sprite(entity_player->texture, position, entity_player->sprite_size, 0);
		}

		// Render entities.
		for (u32 i = 1; i < entity_context->entity_array_count; ++i) {
			Entity *entity = &entity_context->entity_array[i];
			Body *body = &physics_context->body_array[entity->body_id];
			vec3 position = {body->aabb.position[0], body->aabb.position[1], 0};
			render_sprite(entity->texture, position, entity->sprite_size, 0);
		}

		glfwSwapBuffers(render_context->window);
	}
}
