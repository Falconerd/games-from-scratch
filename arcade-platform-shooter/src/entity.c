#include "shared.h"

static Entity_Context *context;

void entity_setup(Entity_Context *physics_context) {
	context = physics_context;
	context->entity_array = calloc(MAX_ENTITIES, sizeof(Entity));
}

u32 entity_create(u32 texture, f32 x, f32 y, f32 collider_half_width, f32 collider_half_height,
                  f32 sprite_width, f32 sprite_height, f32 sprite_offset_x, f32 sprite_offset_y, u32 layer_mask) {
	u32 index = MAX_ENTITIES;
	for (u32 i = 0; i < MAX_ENTITIES; ++i) {
		if (!context->entity_array[i].is_in_use) {
			index = i;
			break;
		}
	}

	if (index == MAX_ENTITIES) {
		error_and_exit(EXIT_FAILURE, "No space for new entities");
	}

	Entity *entity = &context->entity_array[index];
	memset(entity, 0, sizeof(*entity));

	entity->texture = texture;
	entity->aabb.position[0] = x;
	entity->aabb.position[1] = y;
	entity->aabb.half_sizes[0] = collider_half_width;
	entity->aabb.half_sizes[1] = collider_half_height;
	entity->sprite_size[0] = sprite_width;
	entity->sprite_size[1] = sprite_height;
	memcpy(entity->sprite_color, (vec4){1, 1, 1, 1}, sizeof(vec4)); 
	entity->sprite_offset[0] = sprite_offset_x;
	entity->sprite_offset[1] = sprite_offset_y;
	entity->layer_mask = layer_mask;
	entity->is_in_use = 1;

	++context->entity_array_count;

	return index;
}

void entity_destroy(u32 index) {
	context->entity_array[index].is_in_use = 0;
	--context->entity_array_count;
}
