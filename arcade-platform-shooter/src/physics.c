#include "shared.h"

static Physics_Context context = {0};
static Body **body_ptr_array;

Physics_Context *physics_setup(u32 max_bodies, u32 max_static_bodies, u32 max_triggers) {
	context.body_array_max = max_bodies;
	context.body_array = malloc(max_bodies * sizeof(*context.body_array));

	context.static_body_array_max = max_static_bodies;
	context.static_body_array = malloc(max_static_bodies * sizeof(*context.static_body_array));

	context.trigger_array_max = max_triggers;
	context.trigger_array = malloc(max_triggers * sizeof(*context.trigger_array));

	body_ptr_array = malloc(max_bodies * sizeof(void*));

	for (u32 i = 0; i < max_bodies; ++i) {
		body_ptr_array[i] = &context.body_array[i];
	}

	return &context;
}

Hit aabb_intersect_aabb(AABB *self, AABB *other) {
	Hit hit = {0};
	f32 dx = other->position[0] - self->position[0];
	f32 px = other->half_sizes[0] + self->half_sizes[0] - fabs(dx);

	// Must not be intersecting because the distance between the two
	// centre points is greater than the x-axis half sizes of both added
	// together.
	if (px <= 0)
		return hit;

	f32 dy = other->position[1] - self->position[1];
	f32 py = other->half_sizes[1] + self->half_sizes[1] - fabs(dy);

	// Same test as above but on the y-axis.
	if (py <= 0)
		return hit;

	// Calculate how far inside (delta), which side (normal) and the point
	// of contact (position).
	if (px < py) {
		f32 sx = fsign(dx);
		hit.delta[0] = px * sx;
		hit.normal[0] = sx;
		hit.position[0] = self->position[0] + self->half_sizes[0] * sx;
		hit.position[1] = other->position[1];
	} else {
		f32 sy = fsign(dy);
		hit.delta[1] = py * sy;
		hit.normal[1] = sy;
		hit.position[0] = other->position[0];
		hit.position[1] = self->position[1] + self->half_sizes[1] * sy;
	}

	hit.body = (Body*)self;

	return hit;
}

Hit aabb_intersect_segment(AABB *self, vec2 position, vec2 delta, f32 padding_x, f32 padding_y) {
	Hit hit = {0};
	f32 scale_x = 1.0f / delta[0];
	f32 scale_y = 1.0f / delta[1];
	f32 sign_x = fsign(scale_x);
	f32 sign_y = fsign(scale_y);
	f32 near_time_x = (self->position[0] - sign_x * (self->half_sizes[0] + padding_x) - position[0]) * scale_x;
	f32 near_time_y = (self->position[1] - sign_y * (self->half_sizes[1] + padding_y) - position[1]) * scale_y;
	f32 far_time_x = (self->position[0] + sign_x * (self->half_sizes[0] + padding_x) - position[0]) * scale_x;
	f32 far_time_y = (self->position[1] + sign_y * (self->half_sizes[1] + padding_y) - position[1]) * scale_y;

	if (near_time_x > far_time_y || near_time_y > far_time_x) {
		return hit;
	}

	f32 near_time = near_time_x > near_time_y ? near_time_x : near_time_y;
	f32 far_time = far_time_x < far_time_y ? far_time_x : far_time_y;

	if (near_time >= 1 || far_time <= 0) {
		return hit;
	}

	if (isnan(near_time)) {
		return hit;
	}

	hit.time = fclamp(near_time, 0, 1);

	if (near_time_x > near_time_y) {
		hit.normal[0] = -sign_x;
		hit.normal[1] = 0;
	} else {
		hit.normal[0] = 0;
		hit.normal[1] = -sign_y;
	}

	hit.delta[0] = (1.0f - hit.time) * -delta[0];
	hit.delta[1] = (1.0f - hit.time) * -delta[1];
	hit.position[0] = position[0] + delta[0] * hit.time;
	hit.position[1] = position[1] + delta[1] * hit.time;

	hit.body = (Body*)self;

	return hit;
}

Sweep aabb_sweep_aabb(AABB *self, AABB *other, vec2 delta) {
	Sweep sweep = {0};

	if (delta[0] == 0 && delta[1] == 0) {
		sweep.position[0] = other->position[0];
		sweep.position[1] = other->position[1];
		sweep.hit = aabb_intersect_aabb(self, other);
		if (sweep.hit.body == NULL) {
			sweep.time = 1;
		} else {
			sweep.hit.time = 0;
		}

		return sweep;
	}

	sweep.hit = aabb_intersect_segment(self, other->position, delta, other->half_sizes[0], other->half_sizes[1]);

	if (sweep.hit.body != NULL) {
		sweep.time = fclamp(sweep.hit.time - FLT_EPSILON, 0, 1);
		sweep.position[0] = other->position[0] + delta[0] * sweep.time;
		sweep.position[1] = other->position[1] + delta[1] * sweep.time;
		vec2 direction;
		vec2_norm(direction, delta);
		sweep.hit.position[0] = fclamp(
			sweep.hit.position[0] + direction[0] * other->half_sizes[0],
			self->position[0] - self->half_sizes[0],
			self->position[0] + self->half_sizes[0]
		);
		sweep.hit.position[1] = fclamp(
			sweep.hit.position[1] + direction[1] * other->half_sizes[1],
			self->position[1] - self->half_sizes[1],
			self->position[1] + self->half_sizes[1]
		);
	} else {
		sweep.position[0] = other->position[0] + delta[0];
		sweep.position[1] = other->position[1] + delta[1];
		sweep.time = 1;
	}

	return sweep;
}

Sweep aabb_sweep_into(AABB *self, Static_Body *body_array, u32 length, vec2 delta) {
	Sweep nearest = {0};
	nearest.time = 1;
	nearest.position[0] = self->position[0] + delta[0];
	nearest.position[1] = self->position[1] + delta[1];
	for (u32 i = 0; i < length; ++i) {
		Static_Body *static_body = &body_array[i];
		Sweep sweep = aabb_sweep_aabb(&static_body->aabb, self, delta);
		if (sweep.time < nearest.time) {
			nearest = sweep;
		}
	}
	return nearest;
}

Body *physics_body_create(f32 x, f32 y, f32 half_width, f32 half_height) {
	if (context.body_array_count == context.body_array_max) {
		error_and_exit(EXIT_FAILURE, "No bodies left.\n");
	}

	u32 index = context.body_array_count++;
	Body body = {{{x, y}, {half_width, half_height}}};
	context.body_array[index] = body;

	return &context.body_array[index];
}

Static_Body *physics_static_body_create(f32 x, f32 y, f32 half_width, f32 half_height) {
	if (context.static_body_array_count == context.static_body_array_max) {
		error_and_exit(EXIT_FAILURE, "No static bodies left.\n");
	}

	u32 index = context.static_body_array_count++;
	Static_Body static_body = {{{x, y}, {half_width, half_height}}};
	context.static_body_array[index] = static_body;

	return &context.static_body_array[index];
}

Trigger *physics_trigger_create(f32 x, f32 y, f32 half_width, f32 half_height) {
	if (context.trigger_array_count == context.trigger_array_max) {
		error_and_exit(EXIT_FAILURE, "No triggers left.\n");
	}

	u32 index = context.trigger_array_count++;
	Trigger trigger = {{{x, y}, {half_width, half_height}}};
	context.trigger_array[index] = trigger;

	return &context.trigger_array[index];
}

static i32 x_axis_comparator(const void *a, const void *b) {
	AABB aabb_a = (*(Body**)a)->aabb;
	AABB aabb_b = (*(Body**)b)->aabb;
	f32 min_a = aabb_a.position[0] - aabb_a.half_sizes[0];
	f32 min_b = aabb_b.position[0] - aabb_b.half_sizes[0];
	if (min_a < min_b) return -1;
	if (min_a > min_b) return 1;
	return 0;
}

void physics_tick(f32 delta_time) {
	for (u32 i = 0; i < context.body_array_count; ++i) {
		Body *body = &context.body_array[i];

		// Integrate.
		body->velocity[1] += GRAVITY;
		if (body->velocity[1] < TERMINAL_VELOCITY)
			body->velocity[1] = TERMINAL_VELOCITY;
		body->aabb.position[0] += body->velocity[0] * delta_time;
		body->aabb.position[1] += body->velocity[1] * delta_time;

		// Triggers.
		for (u32 j = 0; j < context.static_body_array_count; ++j) {
			Trigger *trigger = &context.trigger_array[j];
			Hit hit = aabb_intersect_aabb(&trigger->aabb, &body->aabb);
			if (hit.body != NULL && trigger->on_trigger != NULL)
				trigger->on_trigger(hit, body);
		}

		// Static collisions.
		u8 was_hit = 0;
		for (u32 j = 0; j < context.static_body_array_count; ++j) {
			Static_Body *static_body = &context.static_body_array[j];
			Hit hit = aabb_intersect_aabb(&static_body->aabb, &body->aabb);
			if (hit.body != NULL) {
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
			}
		}

		if (was_hit == 0) {
			body->is_grounded = 0;
		}
	}
}
