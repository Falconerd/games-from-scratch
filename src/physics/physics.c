#include <stdio.h>
#include <stdlib.h>
#include "physics.h"
#include "physics_internal.h"

static Physics_State physics_state = {0};

Physics_State *physics_init(void) {
    physics_state.bodies = calloc(MAX_BODIES, sizeof(*physics_state.bodies));
    physics_state.body_active = calloc(MAX_BODIES, sizeof(*physics_state.body_active));
    physics_state.body_max = 0xdeadbeef;

    physics_state.static_bodies = calloc(MAX_STATIC_BODIES, sizeof(*physics_state.static_bodies));
    physics_state.static_body_active = calloc(MAX_STATIC_BODIES, sizeof(*physics_state.static_body_active));
    physics_state.static_body_max = 0xdeadbeef;

    physics_collision_init();

    return &physics_state;
}

uint32_t physics_body_create(vec2 pos, vec2 size) {
    uint32_t index = MAX_BODIES;
    for (uint32_t i = 0; i < MAX_BODIES; ++i) {
        if (!physics_state.body_active[i]) {
            index = i;
            break;
        }
    }

    if (index == MAX_BODIES) {
        printf("Max physics bodies reached.\n");
        exit(1);
    }

    AABB *body = (AABB*)&physics_state.bodies[index];
    memset(body, 0, sizeof(*body));

    body->position[0] = pos[0];
    body->position[1] = pos[1];
    body->collider_half_sizes[0] = size[0] * 0.5f;
    body->collider_half_sizes[1] = size[1] * 0.5f;

    physics_state.body_active[index] = 1;

    if (index > physics_state.body_max || physics_state.body_max == 0xdeadbeef) {
        physics_state.body_max = index;
    }

    return index;
}

uint32_t physics_static_body_create(vec2 pos, vec2 size) {
    uint32_t index = MAX_STATIC_BODIES;
    for (uint32_t i = 0; i < MAX_STATIC_BODIES; ++i) {
        if (!physics_state.static_body_active[i]) {
            index = i;
            break;
        }
    }

    if (index == MAX_STATIC_BODIES) {
        printf("Max static physics bodies reached.\n");
        exit(1);
    }

    AABB *static_body = (AABB*)&physics_state.static_bodies[index];
    memset(static_body, 0, sizeof(*static_body));

    static_body->position[0] = pos[0];
    static_body->position[1] = pos[1];
    static_body->collider_half_sizes[0] = size[0] * 0.5f;
    static_body->collider_half_sizes[1] = size[1] * 0.5f;

    physics_state.static_body_active[index] = 1;

    if (index > physics_state.static_body_max || physics_state.static_body_max == 0xdeadbeef) {
        physics_state.static_body_max = index;
    }

    return index;
}


static void physics_integrate(uint32_t index, float delta_time) {
    Body *body = &physics_state.bodies[index];

    body->velocity[1] += GRAVITY;
    if (body->velocity[1] < TERMINAL_VELOCITY) {
        body->velocity[1] = TERMINAL_VELOCITY;
    }

    vec2_add(body->velocity, body->velocity, body->acceleration);

    body->aabb.position[0] += body->velocity[0] * delta_time;
    body->aabb.position[1] += body->velocity[1] * delta_time;
}

void physics_update(float delta_time) {
    if (physics_state.body_max == 0xdeadbeef) {
        return;
    }

    for (uint32_t i = 0; i <= physics_state.body_max; ++i) {
        if (!physics_state.body_active[i]) {
            continue;
        }

        physics_collision_body_body(&physics_state, i);
        physics_integrate(i, delta_time);

        if (physics_state.static_body_max != 0xdeadbeef) {
            physics_collision_body_static(&physics_state, i);
        }

        physics_collision_cleanup();
    }
}
