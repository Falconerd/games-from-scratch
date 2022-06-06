/* Various sources in no particular order:
 * https://noonat.github.io/intersect
 * https://blog.hamaluik.ca/posts/simple-aabb-collision-using-minkowski-difference
 * https://github.com/pgkelley4/line-segments-intersect/blob/master/js/line-segments-intersect.js
 * https://www.youtube.com/watch?v=_g8DLrNyVsQ - Handmade Hero 50
 * https://tavianator.com/2011/ray_box.html
 * https://tavianator.com/2015/ray_box_nan.html
 * https://www.deengames.com/blog/2020/a-primer-on-aabb-collision-resolution.html
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

f32 vec2_sqr_dist(vec2 a, vec2 b) {
	f32 dx = a[0] - b[0];
	f32 dy = a[1] - b[1];
	return dy * dy + dx * dx;
}

bool aabb_intersect_any_static(AABB a, u32 skip) {
	for (u32 i = 0; i < physics_state.body_static_list->len; ++i) {
		Body_Static *b = (Body_Static*)array_list_at(physics_state.body_static_list, i);

		if (skip == i) continue;
		if (!b->is_active) continue;

		AABB c = aabb_minkowski(b->aabb, a);
		vec2 min, max;
		aabb_min_max(min, max, c);

		if (min[0] < 0 && max[0] > 0 && min[1] < 0 && max[1] > 0) {
			return true;
		}
	}

	return false;
}

void stationary_response(Body *a) {
	for (u32 i = 0; i < physics_state.body_static_list->len; ++i) {
		Body_Static *b = array_list_at(physics_state.body_static_list, i);

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
	}
}

bool sweep_static(AABB aabb, vec2 velocity, Hit *sweep_hit) {
	memset(sweep_hit, 0, sizeof(Hit));
	sweep_hit->time = 0xBEEF; // Set to a large number, 2 would probably work as well.
	Hit hit = {0};

	// Only hit the nearest object.
	// This approach has its own issues - such as tunneling on inside corners when at high velocity.
	// High velocity = velocity that would take the moving body > 50% through the static body.
	// For most use-cases it should work fine, though.
	for (u32 i = 0; i < physics_state.body_static_list->len; ++i) {
		Body_Static *b = array_list_at(physics_state.body_static_list, i);

		if (!b->is_active) continue;

		if (ray_intersect_aabb(aabb.position, velocity, aabb_sum(b->aabb, aabb), &hit)) {
			if (hit.time < sweep_hit->time) {
				*sweep_hit = hit;
			} else if (hit.time == sweep_hit->time) {
				if (fabsf(velocity[0]) > fabsf(velocity[1])) {
					if (hit.normal[0] != 0) {
						*sweep_hit = hit;
					}
				} else if (fabsf(velocity[1]) > fabsf(velocity[0])) {
					if (hit.normal[1] != 0) {
						*sweep_hit = hit;
					}
				}
			}
		}
	}

	return sweep_hit->time != 0xBEEF;
}

void sweep_response(Body *a, vec2 vel) {
	Hit h = {0};
	sweep_static(a->aabb, vel, &h);

	if (sweep_static(a->aabb, vel, &h)) {
		a->aabb.position[0] = h.position[0];
		a->aabb.position[1] = h.position[1];

		if (h.normal[0] != 0) {
			a->aabb.position[1] += vel[1];
		}

		if (h.normal[1] > 0) {
			a->is_grounded = true;
			a->aabb.position[0] += vel[0];
		}

		if (h.normal[1] < 0) {
			if (vel[1] > 0) {
				a->aabb.position[0] += vel[0];
			}
		}

		render_line_segment((vec2){
			a->aabb.position[0],
			a->aabb.position[1]
		}, (vec2){
			a->aabb.position[0] + 50 * h.normal[0],
			a->aabb.position[1] + 50 * h.normal[1]
		}, (vec4){1, 1, 1, 1});
	} else {
		a->aabb.position[0] += vel[0];
		a->aabb.position[1] += vel[1];
	}
}


void physics_update(f32 delta_time) {
	for (u32 i = 0; i < physics_state.body_list->len; ++i) {
		Body *a = (Body*)array_list_at(physics_state.body_list, i);

		if (!a->is_active) {
			continue;
		}

		a->velocity[1] += GRAVITY;
		if (TERMINAL_VELOCITY > a->velocity[1]) {
			a->velocity[1] = TERMINAL_VELOCITY;
		}

		a->is_grounded = false;

		vec2 scaled_velocity = { a->velocity[0] * delta_time * TICK, a->velocity[1] * delta_time * TICK };
		for (u32 j = 0; j < ITERATIONS; ++j) {
			sweep_response(a, scaled_velocity);
			stationary_response(a);
		}

		vec2 x;
		vec2_scale(x, a->velocity, delta_time);
		vec2_add(x, a->aabb.position, x);
		render_line_segment(a->aabb.position, x, YELLOW);
	}
}

u32 physics_body_create(vec2 position, vec2 size) {
	Body body = {
		.aabb = {
			.position = { position[0], position[1] },
			.half_size = { size[0] * 0.5, size[1] * 0.5 }
		},
		.velocity = { 0, 0 }
	};

	array_list_append(physics_state.body_list, &body);

	return physics_state.body_list->len - 1;
}

u32 physics_body_static_create(vec2 position, vec2 size) {
	Body_Static body_static = {
		.aabb = {
			.position = { position[0], position[1] },
			.half_size = { size[0] * 0.5, size[1] * 0.5 }
		},

	};

	array_list_append(physics_state.body_static_list, &body_static);

	return physics_state.body_static_list->len - 1;
}

