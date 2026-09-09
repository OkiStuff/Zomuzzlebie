#include "arena.h"
#include <stdint.h>

arena init_arena(size_t bytes)
{
	return (arena)LIST_INIT_WITH_CAPACITY_AND_ELEMENT_SIZE(arena, 1, bytes);
}

arena_allocation arena_alloc(arena* arena, size_t bytes)
{
	list_ensure_capacity(arena, 1, bytes, "arena");
	
	void* ptr = arena + arena->length;
	arena->length += bytes;

	return (arena_allocation){ptr, bytes};
}

void arena_release(arena* arena, size_t bytes)
{
	arena->length -= bytes;
}

void unload_arena(arena* arena);
