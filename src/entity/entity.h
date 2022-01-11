#ifndef ENTITY_H
#define ENTITY_H

#include <stdlib.h>
#include <linmath.h>
#include "../physics/physics.h"

#define MAX_ENTITIES 256

typedef struct entity {
    Body *body;
} Entity;

typedef struct entity_state {
    Entity *entities;
    uint32_t entity_count;
    uint8_t *entity_active;
} Entity_State;

Entity_State *entity_init(void);
uint32_t entity_create(vec2 pos, vec2 collider_half_sizes, Body *bodies);

#endif
