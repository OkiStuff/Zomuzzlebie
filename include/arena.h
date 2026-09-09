#ifndef MUZOMBIE_ARENA_H
#define MUZOMBIE_ARENA_H

#include "list.h"

typedef generic_list arena;

typedef struct arena_allocation
{
	void* buffer;
	size_t length;
} arena_allocation;

arena init_arena(size_t bytes);
arena_allocation arena_alloc(arena* arena, size_t bytes);
void arena_release(arena* arena, size_t bytes);
void unload_arena(arena* arena);

#endif // MUZOMBIE_ARENA_H
