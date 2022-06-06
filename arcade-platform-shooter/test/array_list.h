#include <assert.h>
#include "../src/engine/types.h"
#include "../src/engine/array_list.h"

int array_list(void) {
	Array_List *list = array_list_init(sizeof(u32), 2);

	assert(list->len == 0);
	assert(list->capacity == 2);

	u32 a = 42;
	u32 b = 23;
	u32 c = 48;

	u32 index_a = array_list_append(list, &a);
	u32 index_b = array_list_append(list, &b);
	u32 index_c = array_list_append(list, &c);

	assert(list->len == 3);
	assert(list->capacity == 4);

	assert(((u32*)list->items)[0] == 42);
	assert(((u32*)list->items)[1] == 23);
	assert(((u32*)list->items)[2] == 48);

	array_list_remove(list, 1);

	assert(((u32*)list->items)[1] == 48);
	assert(list->len == 2);
	assert(list->capacity == 4);

	return 0;
}
