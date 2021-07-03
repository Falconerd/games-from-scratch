#include "shared.h"

static Entity_Context context = {0};

Entity_Context *entity_setup() {
	return &context;
}

Entity *entity_create(u32 texture, vec2 position, vec2 body_size, vec2 sprite_offset, vec2 sprite_size, u8 max_health) {
	Entity *entity = NULL;
	for (u32 i = 0; i < MAX_ENTITIES; ++i) {
		if ((context.entities[i].flags & ENTITY_IS_IN_USE) == 0) {
			entity = &context.entities[i];
		}
	}

	if (entity == NULL) {
		error_and_exit(EXIT_FAILURE, "Entity limit reached");
	}

	memset(entity, 0, sizeof(Entity));

	entity->flags |= ENTITY_IS_IN_USE;

	memcpy(entity->body.aabb.min, position, sizeof(vec2));
	entity->body.aabb.max[0] = position[0] + body_size[0];
	entity->body.aabb.max[1] = position[1] + body_size[1];
	entity->texture = texture;
	memcpy(entity->sprite_offset, sprite_offset, sizeof(vec2));
	memcpy(entity->sprite_size, sprite_size, sizeof(vec2));
	entity->health = entity->max_health = max_health;

	return entity;
}

void entity_reset() {
	memset(context.entities, 0, MAX_ENTITIES * sizeof(Entity));
}

void entity_destroy(Entity *entity) {
	entity->flags &= ~ENTITY_IS_IN_USE;
	entity->body.mask = 0;
}
