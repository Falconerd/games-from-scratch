#include "shared.h"

Entity_State entity_state = {0};
static Entity_State *state = &entity_state;

void entity_setup() {
	state->entity_array = calloc(MAX_ENTITIES, sizeof(Entity));
}

u32 entity_create(f32 x, f32 y, f32 collider_half_width, f32 collider_half_height, f32 sprite_width, f32 sprite_height,
				  f32 sprite_offset_x, f32 sprite_offset_y, u32 layer_mask, u32 initial_animation_id) {
	u32 index = MAX_ENTITIES;
	for (u32 i = 0; i < MAX_ENTITIES; ++i) {
		if (!state->entity_array[i].is_in_use) {
			index = i;
			break;
		}
	}

	if (index == MAX_ENTITIES) {
		error_and_exit(EXIT_FAILURE, "No space for new entities");
	}

	Entity *entity = &state->entity_array[index];
	memset(entity, 0, sizeof(*entity));

	entity->animation_id = 0xdeadbeef;
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
	entity->animation_id = initial_animation_id;

	++state->entity_array_count;

	return index;
}

void entity_destroy(u32 index) {
	state->entity_array[index].is_in_use = 0;
	--state->entity_array_count;
}
