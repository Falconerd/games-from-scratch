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

Hit *aabb_intersect_aabb(AABB self, AABB other) {
    Hit *hit = &hits[next_hit_index++];

    float dx = self.position[0] - other.position[0];
    float px = self.collider_half_sizes[0] + other.collider_half_sizes[0] - fabs(dx);

    if (px <= 0) {
        return NULL;
    }

    float dy = self.position[1] - other.position[1];
    float py = self.collider_half_sizes[1] + other.collider_half_sizes[1] - fabs(dy);

    if (py <= 0) {
        return NULL;
    }

    if (px < py) {
        float sx = fsign(dx);
        hit->delta[0] = px * sx;
        hit->normal[0] = sx;
        hit->position[0] = other.position[0] + other.collider_half_sizes[0] * sx;
        hit->position[1] = self.position[1];
    } else {
        float sy = fsign(dy);
        hit->delta[1] = py * sy;
        hit->normal[1] = sy;
        hit->position[0] = self.position[0];
        hit->position[1] = other.position[1] + other.collider_half_sizes[1] * sy;
    }

    return hit;
}

void physics_body_collide_body(Physics_State *physics_state, uint32_t index) {
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

void physics_body_collide_static(Physics_State *physics_state, uint32_t index) {
    Body *self = &physics_state->bodies[index];

    uint8_t was_hit = 0;

    for (uint32_t i = 0; i <= physics_state->static_body_max; ++i) {
        if (!physics_state->static_body_active[i]) {
            continue;
        }

        Static_Body *other = &physics_state->static_bodies[i];

        Hit *hit = aabb_intersect_aabb(self->aabb, other->aabb);

        if (hit) {
            vec2_add(self->aabb.position, self->aabb.position, hit->delta);
            render_quad(hit->position, (vec2){2.f, 2.f}, (vec4){1, 1, 1, 1});

            if (hit->normal[0] == 0 && hit->normal[1] == 1) {
                self->velocity[1] = 0;
            }

            if (hit->normal[1] == -1) {
                self->velocity[1] = 0;
            }

            was_hit = 1;

            if (self->on_collide_static) {
                self->on_collide_static(index, i, *hit);
            }
        }
    }
}
