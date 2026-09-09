#ifndef MUZOMBIE_LIST_H
#define MUZOMBIE_LIST_H

#define STRICT_LIST 0

#include <Muzzle.h>
#include <stdlib.h>
#include "strict_alloc.h"

#define LIST(T) struct\
{\
	T* data;\
	size_t length;\
	size_t capacity;\
}

typedef LIST(void) generic_list;

#ifndef LIST_DEFAULT_INITIAL_CAPACITY
	#define LIST_DEFAULT_INITIAL_CAPACITY 8
#endif

#define LIST_INIT(T) LIST_INIT_WITH_CAPACITY(T, LIST_DEFAULT_INITIAL_CAPACITY)

#define LIST_INIT_EMPTY(T) LIST_INIT_WITH_CAPACITY(T, 0)

#define LIST_INIT_WITH_CAPACITY(T, c) LIST_INIT_WITH_CAPACITY_AND_ELEMENT_SIZE(T, sizeof(T), c)

#define LIST_INIT_WITH_CAPACITY_AND_ELEMENT_SIZE(T, es, c) {\
	.data = STRICT_ALLOC(es * c, #T " list", .allow_empty = MUZZLE_TRUE),\
	.length = 0,\
	.capacity = c\
}

#define LIST_APPEND(T, list, value) LIST_APPEND_WITH_ELEMENT_SIZE(T, list, sizeof(T), value)

#define LIST_APPEND_WITH_ELEMENT_SIZE(T, list, es, value) do\
{\
	list_ensure_capacity((generic_list*)(list), (es), (list)->length + 1, #T);\
	(list)->data[(list)->length++] = value;\
} while (0)\

#define LIST_GET_LAST(list) (list)->data[(list)->length - 1]

#define LIST_FOREACH(list, T, var) for (int i = 0; i < (list)->length; i++) for (T* var = &(list)->data[i]; var != NULL; var = NULL)

void list_ensure_capacity(generic_list* list, size_t element_size, size_t required_capacity, const char* type_name);

#endif // MUZOMBIE_LIST_H
