#include "shared.h"
#include "entity.h"

static Entity_Context context = {0};

Entity_Context *entity_setup() {
	return &context;
}

Entity *entity_create(u32 texture, vec2 offset, vec2 size, Body *body, u8 max_health) {
	if (context.entity_count == MAX_ENTITIES - 1) {
		error_and_exit(EXIT_FAILURE, "Entity limit reached");
	}

	Entity *entity = &context.entities[context.entity_count++];
	entity->texture = texture;
	memcpy(entity->offset, offset, sizeof(vec2));
	memcpy(entity->size, size, sizeof(vec2));
	entity->body = body;
	entity->health = entity->max_health = max_health;

	return entity;
}

void entity_destroy(Entity *entity) {
	Entity *last_entity = &context.entities[--context.entity_count];
	memcpy(entity, last_entity, sizeof(Entity));
}
