#include <stdio.h>
#include <string.h>

#include "../types.h"

#include "../array_list.h"

Array_List *array_list_init(size_t item_size, u32 initial_capacity) {
	Array_List *list = malloc(sizeof(Array_List));

	list->items = malloc(item_size * initial_capacity);
	list->len = 0;
	list->capacity = initial_capacity;
	list->item_size = (u32)item_size;

	return list;
}

u32 array_list_append(Array_List *list, void *data) {
	u32 index = list->len++;

	if (list->len == list->capacity) {
		list->capacity *= 2;
		void *items = realloc(list->items, list->item_size * list->capacity);

		if (!items) {
			printf("No size for reallocation. Exiting.\n");
			exit(1);
		}

		list->items = items;
	}

	u8 *ptr = list->items;
	ptr += index * list->item_size;

	memcpy(ptr, data, list->item_size);

	return index;
}

void array_list_remove(Array_List *list, u32 index) {
	if (list->len == 0) {
		printf("Cannot remove from enmpty list. Exiting.\n");
		exit(1);
	}

	if (list->len == 1) {
		--list->len;
		return;
	}

	--list->len;

	u8 *item_ptr = list->items;
	item_ptr += index * list->item_size;
	u8 *end_ptr = list->items;
	end_ptr += list->len * list->item_size;

	memcpy(item_ptr, end_ptr, list->item_size);
}

void *array_list_at(Array_List *list, u32 index) {
	return (void*)((u8*)list->items+(index * list->item_size));
}

