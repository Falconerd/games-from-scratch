#ifndef PHYSICS_H
#define PHYSICS_H

#include <stdlib.h>
#include <linmath.h>

#define MAX_BODIES 256
#define MAX_STATIC_BODIES 32

#define GRAVITY -20
#define TERMINAL_VELOCITY -200

typedef struct hit Hit;

typedef void (*On_Collide_Function)(uint32_t self_id, uint32_t other_id, Hit hit);
typedef void (*On_Collide_Static_Function)(uint32_t self_id, uint32_t other_id, Hit hit);

struct hit {
    vec2 position;
    vec2 delta;
    vec2 normal;
    float time;
};

typedef struct aabb {
    vec2 position;
    vec2 collider_half_sizes;
} AABB;

typedef struct body {
    AABB aabb;
    vec2 velocity;
    vec2 acceleration;
    On_Collide_Function on_collide;
    On_Collide_Function on_collide_static;
} Body;

typedef struct static_body {
    AABB aabb;
    On_Collide_Function on_collide;
} Static_Body;

typedef struct physics_state {
    Body *bodies;
    uint8_t *body_active;
    uint32_t body_max;

    Static_Body *static_bodies;
    uint8_t *static_body_active;
    uint32_t static_body_max;
} Physics_State;

Physics_State *physics_init(void);
uint32_t physics_body_create(vec2 pos, vec2 size);
uint32_t physics_static_body_create(vec2 pos, vec2 size);
void physics_update(float delta_time);

#endif
