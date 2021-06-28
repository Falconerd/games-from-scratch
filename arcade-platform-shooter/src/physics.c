#include "physics.h"

static Physics_Context context = {0};
static Body *body_ptrs[MAX_OBJECTS] = {0};
static u32 sort_axis = 0;

static void physics_integrate(f32 delta_time) {
	for (u32 i = 0; i < context.body_count; ++i) {
		Body *body = &context.bodies[i];
		if (body->fixed) {
			continue;
		}
		// body->velocity[1] += GRAVITY * delta_time;
		body->aabb.min[0] += body->velocity[0];
		body->aabb.max[0] += body->velocity[0];
		body->aabb.min[1] += body->velocity[1];
		body->aabb.max[1] += body->velocity[1];
	}
}

static u32 aabb_overlap(AABB a, AABB b) {
	if (a.max[0] < b.min[0] || a.min[0] > b.max[0]) return 0;
	if (a.max[1] < b.min[1] || a.min[1] > b.max[1]) return 0;
	return 1;
}

static i32 x_axis_comparator(const void *a, const void *b) {
	f32 min_a = (*(Body**)a)->aabb.min[0];
	f32 min_b = (*(Body**)b)->aabb.min[0];
	if (min_a < min_b) return -1;
	if (min_a > min_b) return 1;
	return 0;
}

f32 fdist(f32 a, f32 b) {
	return fabs(a - b);
}

#define LEFT 1
#define RIGHT 2
#define TOP 4
#define BOTTOM 8

static u32 collision_direction(Body *fixed, Body *unfixed) {
	vec2 velocity = {unfixed->velocity[0], unfixed->velocity[1]};
	vec2 fixed_min = {fixed->aabb.min[0], fixed->aabb.min[1]};
	vec2 fixed_max = {fixed->aabb.max[0], fixed->aabb.max[1]};
	vec2 unfixed_min = {unfixed->aabb.min[0], unfixed->aabb.min[1]};
	vec2 unfixed_max = {unfixed->aabb.max[0], unfixed->aabb.max[1]};

	if (velocity[0] + velocity[1] == 0)
		return 0;

	// probably horizontal
	if (fabs(velocity[0]) > fabs(velocity[1])) {
		if (velocity[0] > 0) {
			if (fixed_min[0] <= unfixed_max[0] && unfixed_min[0] < fixed_min[0]) {
				return LEFT;
			}
		}

		if (velocity[0] < 0) {
			if (fixed_max[0] >= unfixed_min[0] && unfixed_max[0] > fixed_max[0]) {
				return RIGHT;
			}
		}

		if (fixed_max[1] >= unfixed_min[1] && unfixed_max[1] > fixed_max[1]) {
			return TOP;
		}

		if (unfixed_max[1] >= fixed_min[1] && unfixed_min[1] < fixed_min[1]) {
			return BOTTOM;
		}
	}

	// probably vertical
	if (fabs(velocity[0]) < fabs(velocity[1])) {
		if (velocity[1] < 0) {
			if (fixed_max[1] <= unfixed_max[1] && unfixed_max[1] > fixed_max[1]) {
				return TOP;
			}
		}

		if (velocity[1] > 0) {
			if (unfixed_max[1] >= fixed_min[1] && unfixed_min[1] < fixed_min[1]) {
				return BOTTOM;
			}
		}

		if (unfixed_max[0] >= fixed_min[0] && unfixed_min[0] < fixed_min[0]) {
			return LEFT;
		}

		if (unfixed_min[0] <= fixed_max[0] && unfixed_max[0] > fixed_max[0]) {
			return RIGHT;
		}
	}
	
	return 0;
}

static void resolve_collision(Body *a, Body *b) {
	if (a->fixed && b->fixed) {
		return;
	}

	if (a->fixed || b->fixed) {
		Body *fixed = a;
		Body *unfixed = b;
		if (b->fixed) {
			fixed = b;
			unfixed = a;
		}

		if (unfixed->velocity[0] + unfixed->velocity[1] == 0)
			return;

		u32 direction = collision_direction(fixed, unfixed);
		switch (direction) {
		case TOP: {
			f32 difference = fixed->aabb.max[1] - unfixed->aabb.min[1];
			unfixed->aabb.min[1] += difference;
			unfixed->aabb.max[1] += difference;
		} break;
		case BOTTOM: {
			f32 difference = fixed->aabb.min[1] - unfixed->aabb.max[1];
			unfixed->aabb.min[1] += difference;
			unfixed->aabb.max[1] += difference;
		} break;
		case LEFT: {
			f32 difference = fixed->aabb.min[0] - unfixed->aabb.max[0];
			unfixed->aabb.min[0] += difference;
			unfixed->aabb.max[0] += difference;
		} break;
		case RIGHT: {
			f32 difference = fixed->aabb.max[0] - unfixed->aabb.min[0];
			unfixed->aabb.min[0] += difference;
			unfixed->aabb.max[0] += difference;
		} break;
		}
	}
}

static void physics_resolve_collisions() {
	qsort(body_ptrs, context.body_count, sizeof(Body*), x_axis_comparator);
	vec2 sum = {0};
	vec2 sum2 = {0};
	vec2 variance = {0};

	for (u32 i = 0; i < context.body_count; ++i) {
		vec2 aabb_centroid = {
			0.5f * (body_ptrs[i]->aabb.min[0] + body_ptrs[i]->aabb.max[0]),
			0.5f * (body_ptrs[i]->aabb.min[1] + body_ptrs[i]->aabb.max[1])
		};

		sum[0] += aabb_centroid[0];
		sum[1] += aabb_centroid[1];
		sum2[0] += aabb_centroid[0] * aabb_centroid[0];
		sum2[1] += aabb_centroid[1] * aabb_centroid[1];

		for (u32 j = i + 1; j < context.body_count; ++j) {
			if (body_ptrs[j]->aabb.min[0] > body_ptrs[i]->aabb.max[0])
				break;
			if (aabb_overlap(body_ptrs[i]->aabb, body_ptrs[j]->aabb)) {
				// handle collision
				resolve_collision(body_ptrs[i], body_ptrs[j]);
			}
		}

		variance[0] = sum2[0] - sum[0] * sum[0] / MAX_OBJECTS;
		variance[1] = sum2[1] - sum[1] * sum[1] / MAX_OBJECTS;

		sort_axis = variance[1] > variance[0];
	}
}

Physics_Context *physics_setup() {
	for (u32 i = 0; i < MAX_OBJECTS; ++i) {
		body_ptrs[i] = &context.bodies[i];
	}

	return &context;
}

void physics_tick(f32 delta_time) {
	physics_integrate(delta_time);
	physics_resolve_collisions();
}

u32 physics_create_body(vec2 position, vec2 size) {
	if (context.body_count == MAX_OBJECTS) {
		error_and_exit(EXIT_FAILURE, "Body limit reached");
	}
	u8 index = context.body_count++;
	Body body = {0};
	body.aabb.min[0] = position[0];
	body.aabb.min[1] = position[1];
	body.aabb.max[0] = position[0] + size[0];
	body.aabb.max[1] = position[1] + size[1];
	context.bodies[index] = body;
	return index;
}
