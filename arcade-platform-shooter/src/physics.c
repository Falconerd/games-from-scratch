#include "shared.h"

static Physics_Context context = {0};

Physics_Context *physics_setup() {
	context.body_array = malloc(MAX_BODIES * sizeof(*context.body_array));
	context.static_body_array = malloc(MAX_STATIC_BODIES * sizeof(*context.static_body_array));
	context.trigger_array = malloc(MAX_TRIGGERS * sizeof(*context.trigger_array));

	return &context;
}

Hit aabb_intersect_aabb(AABB *self, AABB *other) {
	Hit hit = {0};
	f32 dx = self->position[0] - other->position[0];
	f32 px = self->half_sizes[0] + other->half_sizes[0] - fabs(dx);

	// Must not be intersecting because the distance between the two
	// centre points is greater than the x-axis half sizes of both added
	// together.
	if (px <= 0)
		return hit;

	f32 dy = self->position[1] - other->position[1];
	f32 py = self->half_sizes[1] + other->half_sizes[1] - fabs(dy);

	// Same test as above but on the y-axis.
	if (py <= 0)
		return hit;

	// Calculate how far inside (delta), which side (normal) and the point
	// of contact (position).
	if (px < py) {
		f32 sx = fsign(dx);
		hit.delta[0] = px * sx;
		hit.normal[0] = sx;
		hit.position[0] = other->position[0] + other->half_sizes[0] * sx;
		hit.position[1] = self->position[1];
	} else {
		f32 sy = fsign(dy);
		hit.delta[1] = py * sy;
		hit.normal[1] = sy;
		hit.position[0] = self->position[0];
		hit.position[1] = other->position[1] + other->half_sizes[1] * sy;
	}

	hit.self = (Body*)self;
	hit.other = (Body*)other;

	return hit;
}

Body *physics_body_create(f32 x, f32 y, f32 half_width, f32 half_height) {
	if (context.body_array_count == MAX_BODIES) {
		error_and_exit(EXIT_FAILURE, "No bodies left.\n");
	}

	u32 index = context.body_array_count++;
	memset(context.body_array[index], 0, sizeof(Body));
	Body body = {{{x, y}, {half_width, half_height}}};
	body.id = index;
	context.body_array[index] = body;

	return &context.body_array[index];
}

void physics_body_destroy(u32 index) {
	context.body_array[index] = context.body_array[--context.body_array_count];
}

Static_Body *physics_static_body_create(f32 x, f32 y, f32 half_width, f32 half_height) {
	if (context.static_body_array_count == MAX_STATIC_BODIES) {
		error_and_exit(EXIT_FAILURE, "No static bodies left.\n");
	}

	u32 index = context.static_body_array_count++;
	Static_Body static_body = {{{x, y}, {half_width, half_height}}};
	context.static_body_array[index] = static_body;

	return &context.static_body_array[index];
}

Trigger *physics_trigger_create(f32 x, f32 y, f32 half_width, f32 half_height) {
	if (context.trigger_array_count == MAX_TRIGGERS) {
		error_and_exit(EXIT_FAILURE, "No triggers left.\n");
	}

	u32 index = context.trigger_array_count++;
	Trigger trigger = {{{x, y}, {half_width, half_height}}};
	trigger.id = index;
	context.trigger_array[index] = trigger;

	return &context.trigger_array[index];
}

void physics_tick(f32 delta_time) {
	for (u32 i = 0; i < context.body_array_count; ++i) {
		Body *body = &context.body_array[i];

		// Integrate.
		if (!body->is_kinematic) {
			body->velocity[1] += GRAVITY;
			if (body->velocity[1] < TERMINAL_VELOCITY)
				body->velocity[1] = TERMINAL_VELOCITY;
		}
		body->aabb.position[0] += body->velocity[0] * delta_time;
		body->aabb.position[1] += body->velocity[1] * delta_time;

		// Triggers.
		for (u32 j = 0; j < context.static_trigger_array_count; ++j) {
			Trigger *trigger = &context.trigger_array[j];
			Hit hit = aabb_intersect_aabb(&body->aabb, &trigger->aabb);
			if (hit.self != NULL && trigger->on_trigger != NULL)
				trigger->on_trigger(hit);
		}

		// Static collisions.
		u32 was_hit = 0;
		for (u32 j = 0; j < context.static_body_array_count; ++j) {
			Static_Body *static_body = &context.static_body_array[j];
			Hit hit = aabb_intersect_aabb(&body->aabb, &static_body->aabb);
			if (hit.self != NULL) {
				if (static_body->layer_mask > 0 && (body->layer_mask & static_body->layer_mask) == 0) {
					continue;
				}

				body->aabb.position[0] += hit.delta[0];
				body->aabb.position[1] += hit.delta[1];
#if DEBUG
				render_point(hit.position, (vec4){1, 1, 0, 1});
#endif

				if (hit.normal[0] == 0 && hit.normal[1] == 1) {
					body->is_grounded = 1;
				}
				if (hit.normal[1] == -1) {
					body->velocity[1] = 0;
				}
				was_hit = 1;

				if (body->on_collide_static != NULL)
					body->on_collide_static(hit);
			}
		}

		if (was_hit == 0) {
			body->is_grounded = 0;
		}
	}
}
