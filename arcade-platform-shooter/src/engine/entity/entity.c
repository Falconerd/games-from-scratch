#include <stdio.h>
#include <inttypes.h>
#include "../entity.h"

static Entity_State entity_state;

Entity_State *entity_init() {
	entity_state.entities = calloc(MAX_ENTITIES, sizeof(Entity));
	return &entity_state;
}

static u32 next_entity_index() {
	u32 index = 0xdeadbeef;

	for (u32 i = 0; i < MAX_ENTITIES; ++i) {
		if (!entity_state.entities[i].is_active) {
			index = i;
			break;
		}
	}

	if (index == 0xdeadbeef) {
		printf("No space for new entities. Exiting.\n");
		exit(1);
	}

	return index;
}

u32 entity_create() {
	u32 index = next_entity_index();
	Entity *entity = &entity_state.entities[index];

	memset(entity, 0, sizeof *entity);

	return index;
}

void entity_destroy(u32 index) {
	entity_state.entities[index].is_active = 0;
	--entity_state.entity_count;
}

