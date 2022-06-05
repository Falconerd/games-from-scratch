/* Various sources in no particular order:
 * https://noonat.github.io/intersect
 * https://blog.hamaluik.ca/posts/simple-aabb-collision-using-minkowski-difference
 * https://github.com/pgkelley4/line-segments-intersect/blob/master/js/line-segments-intersect.js
 * https://www.youtube.com/watch?v=_g8DLrNyVsQ - Handmade Hero 50
 * https://tavianator.com/2011/ray_box.html
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

static bool aabb_intersect_aabb(AABB aabb) {
	vec2 min, max;
	aabb_min_max(min, max, aabb);

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

bool ray_intersect_aabb(vec2 p, vec2 d, AABB aabb, Hit *hit) {
	vec2 scale = { 1 / d[0], 1 / d[1] };
	vec2 sign = { fsignf(scale[0]), fsignf(scale[1]) };

	vec2 times_near = {
		(aabb.position[0] - sign[0] * aabb.half_size[0] - p[0]) * scale[0],
		(aabb.position[1] - sign[1] * aabb.half_size[1] - p[1]) * scale[1]
	};

	vec2 times_far = {
		(aabb.position[0] + sign[0] * aabb.half_size[0] - p[0]) * scale[0],
		(aabb.position[1] + sign[1] * aabb.half_size[1] - p[1]) * scale[1]
	};

	if (times_near[0] > times_far[1] || times_near[1] > times_far[0]) {
		return false;
	}

	f32 time_near = times_near[0] > times_near[1] ? times_near[0] : times_near[1];
	f32 time_far = times_far[0] < times_far[1] ? times_far[0] : times_far[1];

	if (time_near >= 1 || time_far <= 0) {
		return false;
	}

	hit->time = fclampf(time_near, 0, 1);

	if (times_near[0] > times_near[1]) {
		hit->normal[0] = -sign[0];
		hit->normal[1] = 0;
	} else {
		hit->normal[0] = 0;
		hit->normal[1] = -sign[1];
	}

	hit->delta[0] = (1 - hit->time) * -d[0];
	hit->delta[1] = (1 - hit->time) * -d[1];
	hit->position[0] = p[0] + d[0] * hit->time;
	hit->position[1] = p[1] + d[1] * hit->time;

	return true;
}

void physics_update(f32 delta_time) {
	for (u32 i = 0; i < physics_state.body_array_count; ++i) {
		Body *a = &physics_state.body_array[i];

		if (!a->is_active) {
			continue;
		}

/*
		a->velocity[1] += GRAVITY;
		if (TERMINAL_VELOCITY > a->velocity[1]) {
			a->velocity[1] = TERMINAL_VELOCITY;
		}
		*/

		vec2 scaled_velocity = { a->velocity[0] * delta_time, a->velocity[1] * delta_time };
		a->is_grounded = false;

		for (u32 j = 0; j < physics_state.body_static_array_count; ++j) {
			Body_Static *b = &physics_state.body_static_array[j];

			if (!b->is_active) {
				continue;
			}

			AABB r = aabb_minkowski(a->aabb, b->aabb);
			render_aabb(&r, YELLOW);

			if (aabb_intersect_aabb(aabb_minkowski(a->aabb, b->aabb))) {
				vec2 pv;
				aabb_penetration_vector(pv, r);
				render_line_segment((vec2){0, 0}, a->aabb.position, GREEN);
				vec2_sub(a->aabb.position, a->aabb.position, pv);
			} else if (scaled_velocity[0] != 0 || scaled_velocity[1] != 0) {
				AABB m = aabb_sum(b->aabb, a->aabb);

				Hit hit;
				if (ray_intersect_aabb(a->aabb.position, scaled_velocity, m, &hit)) {
					printf("KEKW %.2f %2.f %2.f\n", hit.time, hit.position[0], hit.position[1]);
					render_line_segment((vec2){0, 0}, a->aabb.position, PINK);
					a->velocity[0] = 0;
					a->velocity[1] = 0;
					a->aabb.position[0] = hit.position[0];
					a->aabb.position[1] = hit.position[1];
				}
				/*
				vec2 v = { a->velocity[0], a->velocity[1] };
				f32 h = ray_intersection_fraction((vec2){0, 0}, scaled_velocity, aabb_minkowski(a->aabb, b->aabb));
				if (h != INFINITY) {
					printf("h: %.2f\n", h);
					//a->aabb.position[0] = a->aabb.position[0] + scaled_velocity[0] * h;
					//a->aabb.position[1] = a->aabb.position[1] + scaled_velocity[1] * h;
					//return;
					vec2 n = {a->aabb.position[0]+scaled_velocity[0]*h, a->aabb.position[1]+scaled_velocity[1]*h};
					render_line_segment(a->aabb.position, n, RED);
					AABB next = a->aabb;
					next.position[0] += scaled_velocity[0];
					next.position[1] += scaled_velocity[1];
					render_aabb(&next, RED);
				}
				*/
			}

/*
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
			*/
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

