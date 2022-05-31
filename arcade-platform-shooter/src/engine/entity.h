#ifndef ENTITY_H
#define ENTITY_H

#include <stdlib.h>
#include "../../deps/lib/linmath.h"

#define MAX_ENTITIES 256

typedef struct entity {
	u32 body_id;
	u8 is_active;
} Entity;

typedef struct entity_state {
	Entity *entities;
	u32 entity_count;
	u32 entity_max;
} Entity_State;

Entity_State *entity_init(void);
u32 entity_create(void);

#endif
