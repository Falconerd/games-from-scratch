#ifndef ARRAY_LIST_H
#define ARRAY_LIST_H

#include <stdlib.h>
#include <stdbool.h>
#include "./types.h"

typedef struct array_list {
	u32 len;
	u32 capacity;
	u32 item_size;
	void *items;
} Array_List;

Array_List *array_list_init(size_t item_size, u32 initial_capacity);
u32 array_list_append(Array_List *list, void *data);
void array_list_remove(Array_List *list, u32 index);
void *array_list_at(Array_List *list, u32 index);

#endif

