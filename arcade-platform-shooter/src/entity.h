#ifndef src_entity_h_INCLUDED
#define src_entity_h_INCLUDED

#define MAX_ENTITIES 128

typedef struct entity {
	vec3 position;
	vec2 size;
	u32 texture;
} Entity;

typedef struct entity_context {
	Entity entities[MAX_ENTITIES];
	u32 entity_count;
} Entity_Context;

Entity *entity_create();
Entity_Context *entity_setup();

#endif // src/entity_h_INCLUDED

