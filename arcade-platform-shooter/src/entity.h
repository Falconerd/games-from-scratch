#ifndef src_entity_h_INCLUDED
#define src_entity_h_INCLUDED

#define MAX_ENTITIES 128

#include "physics.h"

typedef struct entity {
	u32 texture;
	vec2 offset;
	vec2 size;
	Body *body;
	u8 max_health;
	u8 health;
	u8 is_enemy;
} Entity;

typedef struct entity_context {
	Entity entities[MAX_ENTITIES];
	u32 entity_count;
} Entity_Context;

Entity *entity_create(u32 texture, vec2 offset, vec2 size, Body *body, u8 max_health);
Entity_Context *entity_setup();

#endif

