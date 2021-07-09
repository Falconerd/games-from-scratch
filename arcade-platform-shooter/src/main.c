#include "shared.h"

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

#define PI 3.1415f

int main(void) {
	srand(time(NULL));

	render_context = render_setup(GAME_TITLE);

	Body a = {{{128, 50}, {32, 8}}};
	Body b = {{{128, 50}, {8, 8}}};
	AABB c = {{0, 0}, {8, 8}};
	f32 angle = 0;

	while (!glfwWindowShouldClose(render_context->window)) {
		context.time_now = glfwGetTime();
		context.delta_time = context.time_now - context.time_last_frame;
		context.time_last_frame = context.time_now;

		glfwPollEvents();
		glClearColor(0.0, 0.0, 0.0, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		// Update physics.

		glUseProgram(render_context->shader);
		glUniformMatrix4fv(glGetUniformLocation(render_context->shader, "projection"), 1, GL_FALSE, &render_context->projection[0][0]);

		angle += 0.2f * PI * context.delta_time;
		b.aabb.position[0] = 128 + cosf(angle) * 48.0f;
		b.aabb.position[1] = 50 + sinf(angle * 2.4f) * 12.0f;

		Hit hit = aabb_intersect_aabb(&a.aabb, &b.aabb);

		// Render terrain.
		// Render entities.
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		render_aabb(a.aabb, (vec4){1, 1, 1, 1});
		if (hit.body != NULL) {
			render_aabb(b.aabb, (vec4){1, 0, 0, 1});
			c.position[0] = b.aabb.position[0] + hit.delta[0];
			c.position[1] = b.aabb.position[1] + hit.delta[1];
			render_aabb(c, (vec4){1, 1, 0, 1});
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			render_point(hit.position, (vec4){1, 1, 0, 1});
		} else {
			render_aabb(b.aabb, (vec4){0, 1, 0, 1});
		}
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glfwSwapBuffers(render_context->window);
	}
}
