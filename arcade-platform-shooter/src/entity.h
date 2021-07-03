#ifndef src_entity_h_INCLUDED
#define src_entity_h_INCLUDED

#include "shared.h"

Entity *entity_create(u32 texture, vec2 position, vec2 body_size, vec2 sprite_offset, vec2 sprite_size, u8 max_health);
Entity_Context *entity_setup();
void entity_reset();
void entity_destroy(Entity *entity);

#endif

