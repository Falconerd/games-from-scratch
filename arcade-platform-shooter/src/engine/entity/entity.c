#include <stdio.h>
#include <inttypes.h>
#include "../entity.h"

static Entity_State *entity_state;

Entity_State *entity_state() {
	entity_state->entity_array = calloc(MAX_ENTITIES, sizeof(Entity));
	return &entity_state;
}

static u32 next_entity_index() {
	u32 index = 0xdeadbeef;

	for (u32 i = 0; i < MAX_ENTITIES; ++i) {
		if (!state->entity_array[i].is_in_use) {
			index = i;
			break;
		}
	}

	if (index == 0xdeadbeef) {
		print("No space for new entities. Exiting.\n");
		exit(1);
	}

	return index;
}

u32 entity_create(vec2 pos, vec2 collider_size, vec2 sprite_size, vec2 sprite_offset, u32 layer_mask, u32 initial_animation_id) {
	u32 index = next_entity_index();

	Entity *entity = &state->entity_array[index];
	memset(entity, 0, sizeof(*entity));

	entity->animation_id = 0xdeadbeef;
	memcpy(entity->aabb.position, pos, sizeof vec2);
	memcpy(entity->sprite_size, sprite_size, sizeof vec2);
	memcpy(entity->sprite_offset, sprite_offset, sizeof vec2);
	memcpy(entity->sprite_color, (vec4){1, 1, 1, 1}, sizeof(vec4)); 
	entity->aabb.half_sizes[0] = collider_size[0] * 0.5;
	entity->aabb.half_sizes[1] = collider_size[1] * 0.5;
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

