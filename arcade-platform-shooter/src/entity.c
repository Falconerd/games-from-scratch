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

	u32 id = context.entity_count++;
	Entity *entity = &context.entities[id];
	entity->texture = texture;
	memcpy(entity->offset, offset, sizeof(vec2));
	memcpy(entity->size, size, sizeof(vec2));
	entity->health = entity->max_health = max_health;

	return entity;
}

void entity_reset() {
	memset(context.entities, 0, MAX_ENTITIES * sizeof(Entity));
	context.entity_count = 0;
}
