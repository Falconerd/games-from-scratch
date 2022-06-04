/* Various sources in no particular order:
 * https://noonat.github.io/intersect
 * https://blog.hamaluik.ca/posts/simple-aabb-collision-using-minkowski-difference
 * https://github.com/pgkelley4/line-segments-intersect/blob/master/js/line-segments-intersect.js
 * https://www.youtube.com/watch?v=_g8DLrNyVsQ - Handmade Hero 50
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

static AABB aabb_minkowski(AABB a, AABB b) {
	AABB r;

	r.position[0] = a.position[0] - b.position[0];
	r.position[1] = a.position[1] - b.position[1];
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

static bool ray_intersect_aabb(vec2 p, vec2 d, AABB a, f32 *tmin, vec2 q) {
	vec2 min, max;
	aabb_min_max(min, max, a);

	*tmin = 0;

	f32 tmax = FLT_MAX;

	for (u8 i = 0; i < 3; ++i) {
		if (fabsf(d[i]) < FLT_EPSILON) {
			if (p[i] < min[i] || p[i] > max[i]) return false;
		} else {
			f32 ood = 1 / d[i];
			f32 t0 = (min[i] - p[i]) * ood;
			f32 t1 = (max[i] - p[i]) * ood;
			if (t0 > t1) {
				f32 temp = t0;
				t0 = t1;
				t1 = temp;
			}

			if (t0 > *tmin) *tmin = t0;
			if (t1 > tmax) tmax = t1;
			if (*tmin > tmax) return false;
		}
	}

	q[0] = p[0] + d[0] * *tmin;
	q[1] = p[1] + d[1] * *tmin;

	return true;
}

static f32 cross_product(vec2 a, vec2 b) {
	return a[0] * b[1] - a[1] * b[0];
}

static bool equal_points(vec2 a, vec2 b) {
	return a[0] == b[0] && a[1] == b[1];
}

static bool all_equal(u32 count, ...) {
	va_list ap;
	va_start(ap, count);
	u32 first = va_arg(ap, u32);

	for (u32 i = 0; i < count; ++i) {
		if (va_arg(ap, u32) != first) {
			return false;
		}
	}

	va_end(ap);

	return true;
}

static f32 rifofr(vec2 p, vec2 p2, vec2 q, vec2 q2) {
	// does a->b intersect c->d?
	vec2 r = { p2[0] - p[0], p2[1] - p[1] };
	vec2 s = { q2[0] - q[0], q2[1] - q[1] };

	vec2 qsubp = { q[0] - p[0], q[1] - p[1] };

	f32 numerator = cross_product(qsubp, r);
	f32 denominator = cross_product(r, s);

	if (numerator == 0 && denominator == 0) {
		return INFINITY;
	}

	if (denominator == 0) {
		return INFINITY;
	}

	f32 u = numerator / denominator;
	f32 t = cross_product(qsubp, s) / denominator;

	if ((t >= 0) && (t <= 1) && (u >= 0) && (u <= 1)) {
		return t;
	}

	return INFINITY;
}

static f32 ray_intersection_fraction(vec2 origin, vec2 magnitude, AABB aabb) {
	vec2 end = { origin[0] + magnitude[0], origin[1] + magnitude[1] };

	vec2 min, max;
	aabb_min_max(min, max, aabb);

	f32 min_t = rifofr(origin, end, (vec2){min[0], min[1]}, (vec2){min[0], max[1]});
	f32 x = rifofr(origin, end, (vec2){min[0], max[1]}, (vec2){max[0], max[1]});
	if (x < min_t) min_t = x;
	x = rifofr(origin, end, (vec2){max[0], max[1]}, (vec2){max[0], min[1]});
	if (x < min_t) min_t = x;
	x = rifofr(origin, end, (vec2){max[0], min[1]}, (vec2){min[0], min[1]});
	if (x < min_t) min_t = x;

	return min_t;
}

static bool aabb_sweep_aabb(vec2 r, AABB a, AABB b, vec2 va, vec2 vb) {
	vec2 v = { va[0] - vb[0], va[1] - vb[1] };

	f32 h = ray_intersection_fraction((vec2){0, 0}, v, aabb_minkowski(a, b));

	if (h < INFINITY) {
		return true;
	}

	return false;
}

bool segment_intersect_aabb(vec2 p, vec2 d, AABB aabb, Hit *hit) {
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

	f32 time = fclampf(time_near, 0, 1);

	if (times_near[0] > times_near[1]) {
		hit->normal[0] = -sign[0];
		hit->normal[1] = 0;
	} else {
		hit->normal[0] = 0;
		hit->normal[1] = -sign[1];
	}

	hit->delta[0] = (1 - time) * -d[0];
	hit->delta[1] = (1 - time) * -d[1];
	hit->position[0] = p[0] + d[0] * time;
	hit->position[1] = p[1] + d[1] * time;

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
			} else {
				f32 time;
				vec2 q;
				if (ray_intersect_aabb(a->aabb.position, scaled_velocity, b->aabb, &time, q)) {
					printf("KEKW %.2f\n", time);
					render_line_segment((vec2){0, 0}, a->aabb.position, PINK);
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

