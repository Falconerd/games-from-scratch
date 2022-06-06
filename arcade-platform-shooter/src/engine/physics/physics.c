/* Various sources in no particular order:
 * https://noonat.github.io/intersect
 * https://blog.hamaluik.ca/posts/simple-aabb-collision-using-minkowski-difference
 * https://github.com/pgkelley4/line-segments-intersect/blob/master/js/line-segments-intersect.js
 * https://www.youtube.com/watch?v=_g8DLrNyVsQ - Handmade Hero 50
 * https://tavianator.com/2011/ray_box.html
 * https://tavianator.com/2015/ray_box_nan.html
 * Real-Time Collision Detection by Christer Ericson
 */
#include <float.h>
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

static void aabb_min_max(vec2 min, vec2 max, AABB aabb) {
	min[0] = aabb.position[0] - aabb.half_size[0];
	min[1] = aabb.position[1] - aabb.half_size[1];
	max[0] = aabb.position[0] + aabb.half_size[0];
	max[1] = aabb.position[1] + aabb.half_size[1];
}

AABB aabb_minkowski(AABB a, AABB b) {
	AABB r;

	r.position[0] = a.position[0] - b.position[0];
	r.position[1] = a.position[1] - b.position[1];
	r.half_size[0] = a.half_size[0] + b.half_size[0];
	r.half_size[1] = a.half_size[1] + b.half_size[1];

	return r;
}

AABB aabb_sum(AABB a, AABB b) {
	AABB r;

	r.position[0] = a.position[0];
	r.position[1] = a.position[1];
	r.half_size[0] = a.half_size[0] + b.half_size[0];
	r.half_size[1] = a.half_size[1] + b.half_size[1];

	return r;
}

static bool aabb_intersect_aabb(AABB a, AABB b) {
	vec2 min, max;
	aabb_min_max(min, max, aabb_minkowski(a, b));

	return (min[0] <= 0 && max[0] >= 0 && min[1] <= 0 && max[1] >= 0);
}

static void aabb_penetration_vector(vec2 r, AABB aabb) {
	vec2 min, max;
	aabb_min_max(min, max, aabb);

	f32 min_dist = fabsf(min[0]);
	r[0] = min[0];
	r[1] = 0;

	if (fabsf(max[0]) < min_dist) {
		min_dist = fabsf(max[0]);
		r[0] = max[0];
	}

	if (fabsf(min[1]) < min_dist) {
		min_dist = fabsf(min[1]);
		r[0] = 0;
		r[1] = min[1];
	}

	if (fabsf(max[1]) < min_dist) {
		r[0] = 0;
		r[1] = max[1];
	}
}

bool ray_intersect_aabb(vec2 position, vec2 magnitude, AABB aabb, Hit *hit) {
	vec2 min, max;
	aabb_min_max(min, max, aabb);

	f32 tmin = -INFINITY, tmax = INFINITY;

	for (u8 i = 0; i < 2; ++i) {
		if (magnitude[i] != 0) {
			f32 t1 = (min[i] - position[i]) / magnitude[i];
			f32 t2 = (max[i] - position[i]) / magnitude[i];

			tmin = fmaxf(tmin, fminf(t1, t2));
			tmax = fminf(tmax, fmaxf(t1, t2));
		} else if (position[i] <= min[i] || position[i] >= max[i]) {
			return false;
		}
	}

	hit->position[0] = position[0] + magnitude[0] * tmin;
	hit->position[1] = position[1] + magnitude[1] * tmin;
	hit->delta[0] = (1 - tmin) * -magnitude[0];
	hit->delta[1] = (1 - tmin) * -magnitude[1];

	hit->normal[0] = 0;
	hit->normal[1] = 0;

	f32 dx = hit->position[0] - aabb.position[0];
	f32 px = aabb.half_size[0] - fabsf(dx);
	f32 dy = hit->position[1] - aabb.position[1];
	f32 py = aabb.half_size[1] - fabsf(dy);

	if (px < py) {
		hit->normal[0] = fsignf(dx);
	} else {
		hit->normal[1] = fsignf(dy);
	}

	return tmax > tmin && tmax > 0 && tmin < 1;
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

		Hit hit = {0};

		for (u32 j = 0; j < physics_state.body_static_array_count; ++j) {
			Body_Static *b = &physics_state.body_static_array[j];

			if (!b->is_active) {
				continue;
			}

			if (scaled_velocity[0] == 0 && scaled_velocity[1] == 0) {
				AABB c = aabb_minkowski(b->aabb, a->aabb);
				vec2 min, max;
				aabb_min_max(min, max, c);

				if (min[0] <= 0 && max[0] >= 0 && min[1] <= 0 && max[1] >= 0) {
					vec2 penetration_vector;
					aabb_penetration_vector(penetration_vector, c);

					a->aabb.position[0] += penetration_vector[0];
					a->aabb.position[1] += penetration_vector[1];
					vec2 x;
					vec2_add(x, a->aabb.position, penetration_vector);
					render_line_segment(a->aabb.position, x, GREEN);
				}
			} else if (ray_intersect_aabb(a->aabb.position, scaled_velocity, aabb_sum(b->aabb, a->aabb), &hit)) {
				a->aabb.position[0] = hit.position[0];
				a->aabb.position[1] = hit.position[1];

					//}
					if (hit.normal[0] != 0) {
						scaled_velocity[0] = 0;
					}

					if (hit.normal[1] > 0) {
						a->is_grounded = true;
						if (scaled_velocity[1] < 0) {
							scaled_velocity[1] = 0;
						}
					}

					if (hit.normal[1] < 0) {
						if (scaled_velocity[1] > 0) {
							scaled_velocity[1] = 0;
						}
					}

					render_line_segment((vec2){
						a->aabb.position[0] - a->aabb.half_size[0] * hit.normal[0],
						a->aabb.position[1] - a->aabb.half_size[1] * hit.normal[1]
					}, (vec2){
						a->aabb.position[0] + a->aabb.half_size[0] * hit.normal[0],
						a->aabb.position[1] + a->aabb.half_size[1] * hit.normal[1]
					}, (vec4){0, 1, 0, 0.5});
			}
		}

		a->aabb.position[0] += scaled_velocity[0];
		a->aabb.position[1] += scaled_velocity[1];

		vec2 x;
		vec2_scale(x, a->velocity, delta_time);
		vec2_add(x, a->aabb.position, x);
		render_line_segment(a->aabb.position, x, YELLOW);
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

