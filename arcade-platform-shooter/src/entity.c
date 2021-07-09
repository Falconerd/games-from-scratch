#include "shared.h"

static Entity_Context context = {0};

Entity_Context *entity_setup(u32 max_entities) {
	context.entity_array_max = max_entities;
	context.entity_array = malloc(max_entities * sizeof(Entity));
	return &context;
}

Entity *entity_create(u32 texture, vec2 sprite_offset, vec2 sprite_size) {
	if (context.entity_array_count == context.entity_array_max) {
		error_and_exit(EXIT_FAILURE, "Entity cap reached.");
	}

	u32 index = context.entity_array_count++;
	Entity *entity = &context.entity_array[index];
	entity->texture = texture;
	memcpy(entity->sprite_offset, sprite_offset, sizeof(vec2));
	memcpy(entity->sprite_size, sprite_size, sizeof(vec2));
	return entity;
}
