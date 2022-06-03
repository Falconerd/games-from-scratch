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

u8 aabb_sweep_aabb(AABB a, AABB b, vec2 va, vec2 vb, f32 *tfirst, f32 *tlast, f32 *nx, f32 *ny) {
	f32 dx = a.position[0] - b.position[0];
	f32 px = a.half_size[0] + b.half_size[0] - fabsf(dx);
	f32 dy = a.position[1] - b.position[1];
	f32 py = a.half_size[1] + b.half_size[1] - fabsf(dy);

	*nx = px < py ? fsignf(a.position[0] - b.position[0]) : 0;
	*ny = px >= py ? fsignf(a.position[1] - b.position[1]) : 0;

	vec2 v;
	//vec2_sub(v, vb, va);

	v[0] = vb[0] - va[0];
	v[1] = vb[1] - va[1];

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

		if (*tfirst > *tlast) return 0;
	}

	return 1;
}

void physics_update(f32 delta_time) {
	for (u32 i = 0; i <= physics_state.body_max; ++i) {
		Body *a = &physics_state.bodies[i];

		if (!a->is_active) continue;

		// Static bodies do not collide with each other.
		// They also do not update with gravity, velocity, etc.
		if (a->is_static) {
			continue;
		}

		a->velocity[1] += GRAVITY;
		if (a->velocity[1] < TERMINAL_VELOCITY)
			a->velocity[1] = TERMINAL_VELOCITY;

		if (a->velocity[1] > 0)
			a->is_grounded = false;

		u8 hit_static = 0;

		for (u32 j = 0; j <= physics_state.body_max; ++j) {
			if (i == j) continue;

			Body *b = &physics_state.bodies[j];

			if (!b->is_active) continue;

			f32 tfirst, tlast;
			f32 nx = 0, ny = 0;
			vec2 va, vb;
			vec2_scale(va, a->velocity, delta_time);
			vec2_scale(vb, b->velocity, delta_time);

			if (aabb_sweep_aabb(a->aabb, b->aabb, va, vb, &tfirst, &tlast, &nx, &ny) && b->is_static) {
				hit_static = 1;

				printf("hit_static: t: %2.f %2.f, n: %2.f %2.f\n", tfirst, tlast, nx, ny);

				// If normal is down, we may hit the ground.
				if (ny > 0) {
					if (a->velocity[1] < 0) {
						a->is_grounded = true;
						a->velocity[1] = GRAVITY;
					} else {
						a->is_grounded = false;
						hit_static = false;
					}
				}

				if (ny < 0) {
					if (a->velocity[1] > 0) {
						a->velocity[1] = 0;
						a->aabb.position[1] = (a->aabb.position[1] + va[1] * tfirst) - 1;
					}
				} else {
					f32 tremainder = 1 - tfirst;
					f32 dp = (a->velocity[0] * ny + a->velocity[1] * nx) * tremainder;

					a->aabb.position[0] += va[0] * tfirst + dp * ny * delta_time;
					a->aabb.position[1] += va[1] * tfirst + dp * nx * delta_time;
				}
			}

			if (i == 0) {
			}
		}

		if (!hit_static) {
			a->aabb.position[0] += a->velocity[0] * delta_time;
			a->aabb.position[1] += a->velocity[1] * delta_time;
		}
		
	}
}

static u32 next_body_index() {
	u32 index = 0xdeadbeef;

	for (u32 i = 0; i < MAX_BODIES; ++i) {
		if (!physics_state.bodies[i].is_active) {
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

u32 physics_body_create(vec2 position, vec2 size, u8 is_static) {
	u32 index = next_body_index();

	Body *body = &physics_state.bodies[index];
	body->aabb.position[0] = position[0];
	body->aabb.position[1] = position[1];
	body->aabb.half_size[0] = size[0] * 0.5;
	body->aabb.half_size[1] = size[1] * 0.5;
	body->is_active = 1;
	body->is_static = is_static;
	body->velocity[0] = 0;
	body->velocity[1] = 0;

	if (index > physics_state.body_max) {
		physics_state.body_max = index;
	}

	++physics_state.body_count;

	return index;
}

