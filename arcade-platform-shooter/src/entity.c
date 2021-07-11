#include "shared.h"

static Entity_Context context = {0};

Entity_Context *entity_setup() {
	context.entity_array_max = MAX_ENTITIES;
	context.entity_array = malloc(MAX_ENTITIES * sizeof(Entity));
	return &context;
}

Entity *entity_create(u32 texture, f32 sprite_size_x, f32 sprite_size_y, f32 offset_x, f32 offset_y) {
	if (context.entity_array_count == context.entity_array_max) {
		error_and_exit(EXIT_FAILURE, "Entity cap reached.");
	}

	u32 index = context.entity_array_count++;
	memset(&context.entity_array[index], 0, sizeof(Entity));
	Entity entity = {texture, {offset_x, offset_y}, {sprite_size_x, sprite_size_y}};
	entity.id = index;
	context.entity_array[index] = entity;

	return &context.entity_array[index];
}

void entity_destroy(u32 index) {
	context.entity_array[index] = context.entity_array[--context.entity_array_count];
}
