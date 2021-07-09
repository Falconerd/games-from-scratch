#include "shared.h"

static Physics_Context context = {0};

Physics_Context *physics_setup(u32 max_bodies) {
	context.body_array_max = max_bodies;
	context.body_array = malloc(max_bodies * (sizeof *context.body_array));
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

Hit aabb_intersect_segment(AABB *self, vec2 position, vec2 delta) {
	Hit hit = {0};
	f32 scale_x = 1.0f / delta[0];
	f32 scale_y = 1.0f / delta[1];
	f32 sign_x = fsign(scale_x);
	f32 sign_y = fsign(scale_y);
	f32 near_time_x = (self->position[0] - sign_x * (self->half_sizes[0]) - position[0]) * scale_x;
	f32 near_time_y = (self->position[1] - sign_y * (self->half_sizes[1]) - position[1]) * scale_y;
	f32 far_time_x = (self->position[0] + sign_x * (self->half_sizes[0]) - position[0]) * scale_x;
	f32 far_time_y = (self->position[1] + sign_y * (self->half_sizes[1]) - position[1]) * scale_y;

	if (near_time_x > far_time_y || near_time_y > far_time_x) {
		return hit;
	}

	f32 near_time = near_time_x > near_time_y ? near_time_x : near_time_y;
	f32 far_time = far_time_x < far_time_y ? far_time_x : far_time_y;

	if (near_time >= 1 || far_time <= 0) {
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

	// sweep.hit = aabb_intersect_segment(other->position, delta, other->half_sizes[0], other->half_sizes[1]);
}
