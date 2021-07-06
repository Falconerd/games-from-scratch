#include "shared.h"

// Shared context with entity.
static Entity_Context *context;
static Body *body_ptrs[MAX_ENTITIES] = {0};

f32 squared_distance(vec2 a, vec2 b) {
	return b[1] * b[1] - a[1] * a[1] + b[0] * b[0] - a[0] * a[0];
}

static void physics_integrate(f32 delta_time) {
	for (u32 i = 0; i < MAX_ENTITIES; ++i) {
		Entity *entity = &context->entities[i];
		if ((entity->flags & ENTITY_IS_IN_USE) == 0) {
			continue;
		}
		Body *body = &entity->body;
		if ((body->flags & BODY_IS_FIXED) != 0) {
			continue;
		}

		if ((body->flags & TOP) == 0 && (body->flags & BODY_IS_KINEMATIC) == 0) {
			body->velocity[1] += -GRAVITY * delta_time;
		}

		if (body->velocity[1] < -TERMINAL_VELOCITY) {
			body->velocity[1] = -TERMINAL_VELOCITY;
		}

		body->min[0] += body->velocity[0];
		body->max[0] += body->velocity[0];
		body->min[1] += body->velocity[1];
		body->max[1] += body->velocity[1];
	}
}

static u32 aabb_overlap(vec2 a_min, vec2 a_max, vec2 b_min, vec2 b_max) {
	if (a_max[0] < b_min[0] || a_min[0] > b_max[0]) return 0;
	if (a_max[1] < b_min[1] || a_min[1] > b_max[1]) return 0;
	return 1;
}

static i32 x_axis_comparator(const void *a, const void *b) {
	f32 min_a = (*(Body**)a)->min[0];
	f32 min_b = (*(Body**)b)->min[0];
	if (min_a < min_b) return -1;
	if (min_a > min_b) return 1;
	return 0;
}

f32 fdist(f32 a, f32 b) {
	return fabs(a - b);
}

static u32 collision_direction(Body *fixed, Body *unfixed) {
	vec2 velocity = {unfixed->velocity[0], unfixed->velocity[1]};
	vec2 fixed_min = {fixed->min[0], fixed->min[1]};
	vec2 fixed_max = {fixed->max[0], fixed->max[1]};
	vec2 unfixed_min = {unfixed->min[0], unfixed->min[1]};
	vec2 unfixed_max = {unfixed->max[0], unfixed->max[1]};

	if (velocity[0] + velocity[1] == 0)
		return 16;

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

	return 32;
}

/* Collisions:
	 * Player/Enemy vs Terrain.
 * 
 * Triggers:
	 * Bullet vs Enemy.
         * Bullet vs Terrain.
         * Enemy vs Fire.
 */
static void resolve_collision(Body *body_a, Body *body_b) {
	Entity *a = (Entity*)body_a;
	Entity *b = (Entity*)body_b;

	if ((a->flags & ENTITY_IS_IN_USE) != 0 || (b->flags & ENTITY_IS_IN_USE) != 0) {
		return;
	}

	if ((body_a->flags & BODY_IS_FIXED) != 0 && (body_b->flags & BODY_IS_FIXED) != 0) {
		return;
	}

	printf("%f\n", body_a->min[1]);
}

static u32 bodies_would_overlap(Body *body_a, Body *body_b) {
	vec2 min_a = {body_a->min[0] + body_a->velocity[0], body_a->min[1] + body_a->velocity[1]};
	vec2 max_a = {body_a->max[0] + body_a->velocity[0], body_a->max[1] + body_a->velocity[1]};
	vec2 min_b = {body_b->min[0] + body_b->velocity[0], body_b->min[1] + body_b->velocity[1]};
	vec2 max_b = {body_b->max[0] + body_b->velocity[0], body_b->max[1] + body_b->velocity[1]};
	return aabb_overlap(min_a, max_a, min_b, max_b);
}

static void physics_resolve_collisions() {
	qsort(body_ptrs, MAX_ENTITIES, sizeof(void*), x_axis_comparator);

	for (u32 i = 0; i < MAX_ENTITIES; ++i) {
		for (u32 j = i + 1; j < MAX_ENTITIES; ++j) {
			if (body_ptrs[j]->min[0] > body_ptrs[i]->max[0])
				break;
		}
	}
}

void physics_reset() {
	for (u32 i = 0; i < MAX_ENTITIES; ++i) {
		body_ptrs[i] = &context->entities[i].body;
	}
}

void physics_setup(Entity_Context *entity_context) {
	context = entity_context;
	physics_reset();
}

void physics_tick(f32 delta_time) {
	physics_integrate(delta_time);
	physics_resolve_collisions();
}
