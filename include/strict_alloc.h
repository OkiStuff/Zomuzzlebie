#ifndef MUZOMBIE_STRICT_ALLOC_H
#define MUZOMBIE_STRICT_ALLOC_H

#include <Muzzle.h>

#define STRICT_ALLOC(params, ...) strict_alloc((strict_alloc_parameters){params,__VA_ARGS__})

typedef struct strict_alloc_parameters
{
	size_t bytes;
	const char* purpose;
	mz_boolean allow_empty;
} strict_alloc_parameters;

void* strict_alloc(strict_alloc_parameters params);

#endif // MUZOMBIE_STRICT_ALLOC_H
