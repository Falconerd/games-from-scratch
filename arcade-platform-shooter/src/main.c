#include "shared.h"

typedef struct game_context {
	f32 time_now;
	f32 time_last_frame;
	f32 delta_time;

	f32 spawn_timer;
	f32 box_spawn_timer;
	u8 jump_key_state;
} Game_Context;

static Game_Context context = {0};
static Entity_Context entity_context = {0};
static Render_Context render_context = {0};
static Physics_Context physics_context = {0};

static u32 TERRAIN_TEXTURE;
static u32 PLAYER_TEXTURE;
static u32 BULLET_TEXTURE;
static u32 BOX_TEXTURE;

static u8 PLAYER_MASK = 1;
static u8 ENEMY_MASK = 2;

static void on_fire_trigger(u32 self_id, u32 trigger_id, Hit hit) {
	Entity *self = &entity_context.entity_array[self_id];
	if (self_id == 0) {
		// Player killed, reset level.
	}
	self->aabb.position[1] = 224;
}

static void on_bullet_collide_static(u32 self_id, u32 static_body_id, Hit hit) {
	entity_destroy(self_id);
}

int main(void) {
	srand(time(NULL));

	// Setup contexts.
	entity_setup(&entity_context);
	render_setup(&render_context);
	physics_setup(&physics_context);

	// Setup textures.
	TERRAIN_TEXTURE = render_texture_create("./assets/terrain.png");
	PLAYER_TEXTURE = render_texture_create("./assets/player.png");
	BULLET_TEXTURE = render_texture_create("./assets/bullet.png");
	BOX_TEXTURE = render_texture_create("./assets/box.png");

	// Setup player.
	u32 player_id = entity_create(PLAYER_TEXTURE, 48, 48, 4, 4, 32, 10, -16, -3.5f, PLAYER_MASK);
	Entity *player = &entity_context.entity_array[player_id];

	// Setup terrain.
	// Bottom.
	physics_static_body_create(32, 18, 32, 18);
	physics_static_body_create(90, 10, 26, 10);
	physics_static_body_create(WIDTH - 32, 18, 32, 18);
	physics_static_body_create(WIDTH - 90, 10, 26, 10);
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
	Trigger *trigger = physics_trigger_create(128, 8, 12, 8);
	trigger->on_trigger = on_fire_trigger;

	while (!glfwWindowShouldClose(render_context.window)) {
		context.time_now = glfwGetTime();
		context.delta_time = context.time_now - context.time_last_frame;
		context.time_last_frame = context.time_now;

		// Handle user input.
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
			horizontal_velocity += 100;
			player->is_flipped = 0;
		}

		if (glfwGetKey(render_context.window, GLFW_KEY_R) == GLFW_PRESS) {
			horizontal_velocity -= 100;
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
			u32 bullet_id = entity_create(BULLET_TEXTURE, player->aabb.position[0], player->aabb.position[1], 1.5f, 1.5f, 3, 3, -1.5f, -1.5f, ENEMY_MASK);
			Entity *bullet = &entity_context.entity_array[bullet_id];
			bullet->is_kinematic = 1;
			bullet->velocity[0] = player->is_flipped ? -400 : 400;
			bullet->on_collide_static = on_bullet_collide_static;
		}

		player->velocity[0] = horizontal_velocity;
		player->velocity[1] = vertical_velocity;

		// Update physics.
		physics_tick(context.delta_time, entity_context.entity_array);
		physics_cleanup();

		// Clear screen, etc.
		glfwPollEvents();
		glClearColor(0.0, 0.0, 0.0, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		// Render terrain.
		render_sprite(TERRAIN_TEXTURE, (vec3){0, 0, 0}, (vec2){256, 224}, 0);

		// Render entities.
		for (u32 i = 0; i < MAX_ENTITIES; ++i) {
			Entity *entity = &entity_context.entity_array[i];
			if (entity->is_in_use) {
				vec3 position = {entity->aabb.position[0] + entity->sprite_offset[0],
						 entity->aabb.position[1] + entity->sprite_offset[1], 0};
				render_sprite(entity->texture, position, entity->sprite_size, entity->is_flipped);
			}
		}

#if DEBUG
		// Render entity colliders.
		for (u32 i = 0; i < MAX_ENTITIES; ++i) {
			Entity *entity = &entity_context.entity_array[i];
			if (entity->is_in_use) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				render_aabb(entity->aabb, (vec4){0, 1, 0, 1});
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
		}

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
