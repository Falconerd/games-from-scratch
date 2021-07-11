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

static u32 TERRAIN_TEXTURE;
static u32 PLAYER_TEXTURE;

static u8 PLAYER_MASK = 1;
static u8 ENEMY_MASK = 2;

static void on_fire_trigger(Hit hit, Body *other) {
	printf("%p vs %p\n", (void*)hit.body, (void*)other);
}

int main(void) {
	srand(time(NULL));

	// Setup contexts.
	physics_context = physics_setup(256, 16, 10);
	render_context = render_setup(GAME_TITLE);
	entity_context = entity_setup(256);

	// Setup textures.
	TERRAIN_TEXTURE = render_texture_create("./assets/terrain.png");
	PLAYER_TEXTURE = render_texture_create("./assets/player.png");

	// Setup player.
	Entity *player_entity = entity_create(PLAYER_TEXTURE, 32, 10, -16, -3.5f);
	Body *player_body = physics_body_create(48, 112, 4, 4);
	player_entity->body_id = player_body->id;
	player_body->layer_mask |= PLAYER_MASK;

	// Setup terrain.
	// Bottom.
	physics_static_body_create(32, 18, 32, 18);
	physics_static_body_create(88, 10, 24, 10);
	physics_static_body_create(WIDTH - 32, 18, 32, 18);
	physics_static_body_create(WIDTH - 88, 10, 24, 10);
	// Top.
	physics_static_body_create(56, HEIGHT - 6.5f, 56, 6.5f);
	physics_static_body_create(200, HEIGHT - 6.5f, 56, 6.5f);
	// Left.
	physics_static_body_create(6.5f, HEIGHT / 2, 6.5f, HEIGHT / 2);
	// Right.
	physics_static_body_create(WIDTH - 6.5f, HEIGHT / 2, 6.5f, HEIGHT / 2);
	// Bottom platform.
	physics_static_body_create(128, 71, 64, 6.5f);
	// Top platform.
	physics_static_body_create(128, 167, 64, 6.5f);
	// Left platform.
	physics_static_body_create(32, 119, 32, 6.5f);
	// Right platform.
	physics_static_body_create(WIDTH - 32, 119, 32, 6.5f);
	// Top player-only collisions.
	Static_Body *static_body = physics_static_body_create(128, HEIGHT - 6.5f, 16, 6.5f);
	static_body->layer_mask |= PLAYER_MASK;

	// Setup fire trigger.
	Trigger *trigger = physics_trigger_create(128, 8, 16, 8);
	trigger->on_trigger = on_fire_trigger;

	while (!glfwWindowShouldClose(render_context->window)) {
		context.time_now = glfwGetTime();
		context.delta_time = context.time_now - context.time_last_frame;
		context.time_last_frame = context.time_now;

		// Handle user input.
		f32 horizontal_velocity = 0;
		f32 vertical_velocity = player_body->velocity[1];

		if (glfwGetKey(render_context->window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(render_context->window, GLFW_TRUE);

		// Variable jump height.
		if (glfwGetKey(render_context->window, GLFW_KEY_F) == GLFW_RELEASE && context.jump_key_state == GLFW_PRESS) {
			vertical_velocity *= 0.5f;
			context.jump_key_state = GLFW_RELEASE;
		}

		if (glfwGetKey(render_context->window, GLFW_KEY_T) == GLFW_PRESS) {
			horizontal_velocity += 100;
		}

		if (glfwGetKey(render_context->window, GLFW_KEY_R) == GLFW_PRESS) {
			horizontal_velocity -= 100;
		}

		if (glfwGetKey(render_context->window, GLFW_KEY_F) == GLFW_PRESS) {
			if (player_body->is_grounded) {
				player_body->is_grounded = 0;
				context.jump_key_state = GLFW_PRESS;
				vertical_velocity = 500;
			}
		}

		player_body->velocity[0] = horizontal_velocity;
		player_body->velocity[1] = vertical_velocity;

		// Update physics.
		physics_tick(context.delta_time);

		// Clear screen, etc.
		glfwPollEvents();
		glClearColor(0.0, 0.0, 0.0, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		// Render terrain.
		render_sprite(TERRAIN_TEXTURE, (vec3){0, 0, 0}, (vec2){256, 224}, 0);

		// Render entities.
		for (u32 i = 0; i < entity_context->entity_array_count; ++i) {
			Entity *entity = &entity_context->entity_array[i];
			Body *body = &physics_context->body_array[entity->body_id];
			vec3 position = {body->aabb.position[0] + entity->sprite_offset[0], body->aabb.position[1] + entity->sprite_offset[1], 0};
			render_sprite(entity->texture, position, entity->sprite_size, 0);
		}

#if DEBUG
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

		for (u32 i = 0; i < physics_context->trigger_array_count; ++i) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			render_aabb(physics_context->trigger_array[i].aabb, (vec4){1, 1, 0, 1});
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
#endif

		glfwSwapBuffers(render_context->window);
	}
}
