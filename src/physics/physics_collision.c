#include <stdlib.h>
#include "physics.h"
#include "physics_internal.h"
#include "../util/util.h"
#include "../render/render.h"

static Hit *hits;
static uint32_t next_hit_index = 0;

void physics_collision_init(void) {
    hits = calloc(MAX_BODIES * MAX_BODIES, sizeof(*hits));
}

static Hit *aabb_intersect_aabb(AABB self, AABB other) {
    Hit *hit = &hits[next_hit_index++];

    float dx = self.position[0] - other.position[0];
    float px = self.half_size[0] + other.half_size[0] - fabs(dx);

    // Must not be intersecting because the distance between the two
    // centre points is greater than the x-axis half sizes of both
    // added together.
    if (px <= 0) {
        return NULL;
    }

    float dy = self.position[1] - other.position[1];
    float py = self.half_size[1] + other.half_size[1] - fabs(dy);

    // Same test as above but on the y-axis.
    if (py <= 0) {
        return NULL;
    }

    // Calculate how far inside (delta), which side (normal) and the point
    // of contact (position).
    if (px < py) {
        float sx = fsign(dx);
        hit->delta[0] = px * sx;
        hit->normal[0] = sx;
        hit->position[0] = other.position[0] + other.half_size[0] * sx;
        hit->position[1] = self.position[1];
    } else {
        float sy = fsign(dy);
        hit->delta[1] = py * sy;
        hit->normal[1] = sy;
        hit->position[0] = self.position[0];
        hit->position[1] = other.position[1] + other.half_size[1] * sy;
    }

    return hit;
}

static uint8_t moving_aabb_intersect_aabb(AABB a, AABB b, vec2 va, vec2 vb, float *tfirst, float *tlast) {
    Hit *hit = aabb_intersect_aabb(a, b);

    if (hit) {
        return 1;
    }

    hit = &hits[next_hit_index++];

    vec2 velocity;
    vec2_sub(velocity, vb, va);

    // Early exit if no velocity.
    if (velocity[0] == 0.f && velocity[1] == 0.f) {
        return 0;
    }

    // My pleb checks.
    AABB s;

    s.half_size[0] = va[0] * 0.5f + a.half_size[0];
    s.half_size[1] = va[1] * 0.5f + a.half_size[1];
    s.position[0] = a.position[0] + va[0] * 0.5f;
    s.position[1] = a.position[1] + va[1] * 0.5f;

    if (!aabb_intersect_aabb(s, b)) {
        return 0;
    }

    vec2 a_min, a_max, b_min, b_max;
    vec2_sub(a_min, a.position, a.half_size);
    vec2_add(a_max, a.position, a.half_size);
    vec2_sub(b_min, b.position, b.half_size);
    vec2_add(b_max, b.position, b.half_size);

    *tfirst = 0.f;
    *tlast = 1.f;

    for (uint32_t i = 0; i < 2; ++i) {
        if (velocity[i] < 0.f) {
            if (b_max[i] < a_min[i]) return 0;
            if (a_max[i] < b_min[i]) {
                *tfirst = fmaxf((a_max[i] - b_min[i]) / velocity[i], *tfirst);
            }
            if (b_max[i] > a_min[i]) {
                *tlast = fminf((a_min[i] - b_max[i]) / velocity[i], *tlast);
            }
        }

        if (velocity[i] > 0.f) {
            if (b_min[i] > a_max[i]) return 0;
            if (b_max[i] < a_min[i]) {
                *tfirst = fmaxf((a_min[i] - b_max[i]) / velocity[i], *tfirst);
            }
            if (a_max[i] > b_min[i]) {
                *tlast = fminf((a_max[i] - b_min[i]) / velocity[i], *tlast);
            }
        }

        if (*tfirst > *tlast) {
            return 0;
        }
    }

    return 1;
}

void physics_collision_body_body(Physics_State *physics_state, uint32_t index) {
    Body *self = &physics_state->bodies[index];

    for (uint32_t i = 0; i <= physics_state->body_max; ++i) {
        if (i == index || !physics_state->body_active[i]) {
            continue;
        }

        Body *other = &physics_state->bodies[i];
        Hit *hit = aabb_intersect_aabb(self->aabb, other->aabb);

        if (hit && self->on_collide) {
            self->on_collide(index, i, *hit);
        }
    }
}

void physics_collision_body_static(Physics_State *physics_state, uint32_t index) {
    Body *self = &physics_state->bodies[index];

    uint8_t hit_left = 0;
    uint8_t hit_right = 0;
    uint8_t hit_top = 0;
    uint8_t hit_bottom = 0;

    uint8_t did = 0;

    for (uint32_t i = 0; i <= physics_state->static_body_max; ++i) {
        if (!physics_state->static_body_active[i]) {
            continue;
        }

        Static_Body *other = &physics_state->static_bodies[i];
        Hit *hit = aabb_intersect_aabb(self->aabb, other->aabb);
        float tf, tl;
        uint8_t x = moving_aabb_intersect_aabb(self->aabb, other->aabb, self->velocity, (vec2){ 0.f, 0.f }, &tf, &tl);

        if (x && index == 0) {
            did = 1;
            printf("Collided with [%u]! %.2f %.2f (%.2f, %.2f)\n", i, tf, tl, self->velocity[0], self->velocity[1]);
            physics_state->debug_aabb.position[0] = self->aabb.position[0];
            physics_state->debug_aabb.position[1] = self->aabb.position[1];
            physics_state->debug_aabb.half_size[0] = self->aabb.half_size[0];
            physics_state->debug_aabb.half_size[1] = self->aabb.half_size[1];
        }

        if (hit) {
            if (hit->normal[0] > 0) {
                hit_left = 1;
            } else if (hit->normal[0] < 0) {
                hit_right = 1;
            }

            if (hit->normal[1] < 0) {
                hit_top = 1;
            } else if (hit->normal[1] > 0) {
                hit_bottom = 1;
            }

            AABB aabb = {{0, 0}, {0, 0}};
            aabb.half_size[0] = self->aabb.half_size[0];
            aabb.half_size[1] = self->aabb.half_size[1];
            vec2_add(aabb.position, self->aabb.position, hit->delta);
            render_aabb(&aabb, (vec4){1, 0, 1, 1});

            if (self->on_collide_static) {
                self->on_collide_static(index, i, *hit);
            }
            render_quad(hit->position, (vec2){2.f, 2.f}, (vec4){1, 1, 1, 1});

            // physics_state->debug_aabb.position[0] = hit->position[0];
            // physics_state->debug_aabb.position[1] = hit->position[1];
            // physics_state->debug_aabb.half_size[0] = self->aabb.half_size[0];
            // physics_state->debug_aabb.half_size[1] = self->aabb.half_size[1];
        }
    }

    self->colliding_left = hit_left;
    self->colliding_right = hit_right;
    self->colliding_top = hit_top;
    self->colliding_bottom = hit_bottom;

    if (did)
        printf("---\n");
}

void physics_collision_cleanup(void) {
    memset(hits, 0, next_hit_index * sizeof(*hits));
    next_hit_index = 0;
}
