#include "../physics.h"
#include "physics_internal.h"

// NOTE: DELETE ME
#include <stdio.h>
#include "../render.h"
// -----

#include "../types.h"
#include "../util.h"

static Physics_State physics_state;

Physics_State *physics_init() {
	physics_init_bodies(&physics_state);

	return &physics_state;
}

u8 aabb_intersect_aabb(AABB a, AABB b) {
	for (u8 i = 0; i < 2; ++i) {
		if (a.position[i] + a.half_size[i] < b.position[i] - b.half_size[i] || a.position[i] - a.half_size[i] > b.position[i] + b.half_size[i]) {
			return 0;
		}
	}
	return 1;
}

u8 aabb_sweep_aabb(AABB a, AABB b, vec2 va, vec2 vb, f32 *tfirst, f32 *tlast, vec2 normal) {
	f32 dx = a.position[0] - b.position[0];
	f32 px = a.half_size[0] + b.half_size[0] - fabsf(dx);
	f32 dy = a.position[1] - b.position[1];
	f32 py = a.half_size[1] + b.half_size[1] - fabsf(dy);

	normal[0] = px < py ? fsignf(a.position[0] - b.position[0]) : 0;
	normal[1] = px >= py ? fsignf(a.position[1] - b.position[1]) : 0;

	if (aabb_intersect_aabb(a, b)) {
		*tfirst = *tlast = 0;
		return 1;
	}

	vec2 v;
	vec2_sub(v, vb, va);

	*tfirst = 0;
	*tlast = 1;

	vec2 amin, amax, bmin, bmax;
	vec2_sub(amin, a.position, a.half_size);
	vec2_sub(bmin, b.position, b.half_size);
	vec2_add(amax, a.position, a.half_size);
	vec2_add(bmax, b.position, b.half_size);

	for (u8 i = 0; i < 2; ++i) {
		if (v[i] < 0) {
			if (bmax[i] < amin[i]) return 0;
			if (amax[i] < bmin[i]) *tfirst = fmaxf((amax[i] - bmin[i]) / v[i], *tfirst);
			if (bmax[i] > amin[i]) *tlast = fminf((amin[i] - bmax[i]) / v[i], *tlast);
		}

		if (v[i] > 0) {
			if (bmin[i] > amax[i]) return 0;
			if (bmax[i] < amin[i]) *tfirst = fmaxf((amin[i] - bmax[i]) / v[i], *tfirst);
			if (amax[i] > bmin[i]) *tlast = fminf((amax[i] - bmin[i]) / v[i], *tlast);
		}

		if (v[i] == 0 && normal[i] != 0) {
			return 0;
		}

		if (*tfirst > *tlast) return 0;
	}

	return 1;
}

void physics_update(f32 delta_time) {
	for (u32 i = 0; i < physics_state.body_array_count; ++i) {
		Body *a = &physics_state.body_array[i];

		if (!a->is_active) {
			continue;
		}

		a->velocity[1] += GRAVITY;
		if (TERMINAL_VELOCITY > a->velocity[1]) {
			a->velocity[1] = TERMINAL_VELOCITY;
		}

		vec2 scaled_velocity = { a->velocity[0] * delta_time, a->velocity[1] * delta_time };
		a->is_grounded = false;

		for (u32 j = 0; j < physics_state.body_static_array_count; ++j) {
			Body_Static *b = &physics_state.body_static_array[j];

			if (!b->is_active) {
				continue;
			}

			f32 tfirst, tlast;
			vec2 normal;

			u8 hit = aabb_sweep_aabb(a->aabb, b->aabb, scaled_velocity, (vec2){0, 0}, &tfirst, &tlast, normal);

			if (hit) {
				a->aabb.position[0] = a->aabb.position[0] + scaled_velocity[0] * tfirst;
				a->aabb.position[1] = a->aabb.position[1] + scaled_velocity[1] * tfirst;

				if (normal[0] != 0) {
					a->velocity[0] = 0;
				}

				if (normal[1] > 0) {
					a->is_grounded = true;
					if (a->velocity[1] < 0) {
						a->velocity[1] = 0;
					}
				}

				if (normal[1] < 0) {
					if (a->velocity[1] > 0) {
						a->velocity[1] = 0;
					}
				}
			}
		}

		vec2 new_velocity = { a->velocity[0] * delta_time, a->velocity[1] * delta_time };
		vec2_add(a->aabb.position, a->aabb.position, new_velocity);
	}
}

static u32 next_body_index() {
	u32 index = 0xdeadbeef;

	for (u32 i = 0; i < BODY_ARRAY_MAX; ++i) {
		if (!physics_state.body_array[i].is_active) {
			index = i;
			break;
		}
	}

	if (index == 0xdeadbeef) {
		printf("No space for new bodies! Exiting.\n");
		exit(1);
	}

	return index;
}

static u32 next_body_static_index() {
	u32 index = 0xdeadbeef;

	for (u32 i = 0; i < BODY_ARRAY_MAX; ++i) {
		if (!physics_state.body_static_array[i].is_active) {
			index = i;
			break;
		}
	}

	if (index == 0xdeadbeef) {
		printf("No space for new static bodies! Exiting.\n");
		exit(1);
	}

	return index;
}

u32 physics_body_create(vec2 position, vec2 size) {
	u32 index = next_body_index();

	Body *body = &physics_state.body_array[index];
	body->aabb.position[0] = position[0];
	body->aabb.position[1] = position[1];
	body->aabb.half_size[0] = size[0] * 0.5;
	body->aabb.half_size[1] = size[1] * 0.5;
	body->is_active = 1;
	body->velocity[0] = 0;
	body->velocity[1] = 0;

	if (index > physics_state.body_array_max) {
		physics_state.body_array_max = index;
	}

	++physics_state.body_array_count;

	return index;
}

u32 physics_body_static_create(vec2 position, vec2 size) {
	u32 index = next_body_static_index();

	Body_Static *body = &physics_state.body_static_array[index];
	body->aabb.position[0] = position[0];
	body->aabb.position[1] = position[1];
	body->aabb.half_size[0] = size[0] * 0.5;
	body->aabb.half_size[1] = size[1] * 0.5;
	body->is_active = 1;

	if (index > physics_state.body_static_array_max) {
		physics_state.body_static_array_max = index;
	}

	++physics_state.body_static_array_count;

	return index;
}

