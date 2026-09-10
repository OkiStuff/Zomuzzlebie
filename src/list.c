#include <Muzzle.h>
#include "list.h"

#ifdef MUZOMBIE_DEBUG_BUILD
#include "helpers.h"
#endif

void list_ensure_capacity(generic_list* list, size_t element_size, size_t required_capacity, const char* type_name)
{
	if (list->capacity >= required_capacity)
	{
		return;
	}

#ifdef MUZOMBIE_DEBUG_BUILD
	size_t old_capacity = list->capacity;
#endif

	list->capacity = list->capacity == 0 ? LIST_DEFAULT_INITIAL_CAPACITY : required_capacity * 1.5;
	list->data = MZ_REALLOC(list->data, list->capacity * element_size);

	if (list->data == NULL)
	{
		mz_log_status_formatted(LOG_STATUS_FATAL_ERROR, "Failed to allocate %zu bytes for %s list", list->capacity * element_size, type_name != NULL ? type_name : "a");
		__builtin_unreachable();
	}

#ifdef MUZOMBIE_DEBUG_BUILD
	mz_log_status_formatted(LOG_STATUS_INFO, "Grew %s list to a capacity of %zu elements (growth_rate=%.1fx, bytes=%zu)", list->capacity, (float)(list->capacity) / MAX(old_capacity, 1), required_capacity * element_size);
#endif
}

void unload_list(generic_list* list)
{
	MZ_FREE(list->data);
	list->data = NULL;
	list->length = 0;
	list->capacity = 0;
}
