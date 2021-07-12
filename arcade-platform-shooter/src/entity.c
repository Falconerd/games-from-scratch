#include "shared.h"

static u8 *entity_context_arena = NULL;
static Entity_Context entity_context = {0};

Entity_Context *entity_setup() {
	entity_context_arena = malloc(MAX_ENTITIES * sizeof(Entity_Size));
	entity_context.aabb_array = (AABB*)entity_context_arena;
	entity_context.velocity_array = (vec2*)(entity_context.aabb_array + MAX_ENTITIES);
	entity_context.texture_array = (u32*)(entity_context.velocity_array + MAX_ENTITIES);
	entity_context.sprite_size_array = (vec2*)(entity_context.texture_array + MAX_ENTITIES);
	entity_context.sprite_offset_array = (vec2*)(entity_context.sprite_size_array + MAX_ENTITIES);
	entity_context.on_collide_array = (On_Collide_Function*)(entity_context.sprite_offset_array + MAX_ENTITIES);
	entity_context.on_collide_static_array = (On_Collide_Static_Function*)(entity_context.on_collide_array + MAX_ENTITIES);
	entity_context.is_in_use_array = (u8*)(entity_context.on_collide_static_array + MAX_ENTITIES);
	entity_context.is_flipped_array = (u8*)(entity_context.is_in_use_array + MAX_ENTITIES);
	entity_context.is_grounded_array = (u8*)(entity_context.is_flipped_array + MAX_ENTITIES);
	entity_context.is_kinematic_array = (u8*)(entity_context.is_grounded_array + MAX_ENTITIES);
	entity_context.layer_mask_array = (u8*)(entity_context.is_kinematic_array + MAX_ENTITIES);
	return &entity_context;
}

u32 entity_create(u32 texture, f32 x, f32 y, f32 collider_half_width, f32 collider_half_height,
                  f32 sprite_width, f32 sprite_height, f32 sprite_offset_x, f32 sprite_offset_y, u32 layer_mask) {
	u32 index = MAX_ENTITIES;
	for (u32 i = 0; i < MAX_ENTITIES; ++i) {
		if (!entity_context.is_in_use_array[i]) {
			index = i;
			break;
		}
	}

	if (index == MAX_ENTITIES) {
		error_and_exit(EXIT_FAILURE, "No space for new entities");
	}

	entity_context.is_in_use_array[index] = 1;

	entity_context.texture_array[index] = texture;
	entity_context.aabb_array[index].position[0] = x;
	entity_context.aabb_array[index].position[1] = y;
	entity_context.aabb_array[index].half_sizes[0] = collider_half_width;
	entity_context.aabb_array[index].half_sizes[1] = collider_half_height;
	entity_context.sprite_size_array[index][0] = sprite_width;
	entity_context.sprite_size_array[index][1] = sprite_height;
	entity_context.sprite_offset_array[index][0] = sprite_offset_x;
	entity_context.sprite_offset_array[index][1] = sprite_offset_y;
	entity_context.layer_mask_array[index] = layer_mask;

	entity_context.velocity_array[index][0] = 0;
	entity_context.velocity_array[index][1] = 0;
	entity_context.on_collide_array[index] = NULL;
	entity_context.on_collide_static_array[index] = NULL;
	entity_context.is_flipped_array[index] = 0;
	entity_context.is_grounded_array[index] = 0;
	entity_context.is_kinematic_array[index] = 0;

	return index;
}

void entity_destroy(u32 index) {
	entity_context.is_in_use_array[index] = 0;
}
