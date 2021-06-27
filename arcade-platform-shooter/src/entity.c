#include "shared.h"
#include "entity.h"

static Entity_Context context = {0};

Entity_Context *entity_setup() {
	return &context;
}

Entity *entity_create() {
	if (context.entity_count == MAX_ENTITIES - 1) {
		error_and_exit(EXIT_FAILURE, "Entity limit reached");
	}

	Entity *entity = &context.entities[context.entity_count++];
	memset(entity, 0, sizeof(Entity));
	entity->texture = 0xdeadbeef;

	return entity;
}

void entity_destroy(Entity *entity) {
	Entity *last_entity = &context.entities[--context.entity_count];
	memcpy(entity, last_entity, sizeof(Entity));
}
