#include <stdio.h>
#include "entity.h"

static Entity_State entity_state = {0};

Entity_State *entity_init(void) {
    entity_state.entities = calloc(MAX_ENTITIES, sizeof(*entity_state.entities));
    entity_state.entity_active = calloc(MAX_ENTITIES, sizeof(*entity_state.entity_active));

    return &entity_state;
}

uint32_t entity_create(vec2 pos, vec2 collider_size, Body *bodies) {
    uint32_t index = MAX_ENTITIES;

    for (uint32_t i = 0; i < MAX_ENTITIES; ++i) {
        if (!entity_state.entity_active[i]) {
            index = i;
            break;
        }
    }

    if (index == MAX_ENTITIES) {
        printf("No space for new entities.\n");
        exit(1);
    }

    Entity *entity = &entity_state.entities[index];
    memset(entity, 0, sizeof(*entity));

    entity->body = &bodies[physics_body_create(pos, collider_size)];

    ++entity_state.entity_count;

    entity_state.entity_active[index] = 1;

    return index;
}

void entity_destroy(uint32_t index) {
    entity_state.entity_active[index] = 0;
    --entity_state.entity_count;
}
