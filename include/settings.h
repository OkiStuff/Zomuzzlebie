#ifndef MUZOMBIE_DATASET_H
#define MUZOMBIE_DATASET_H

#include "arena.h"
#include "types.h"
#include "helpers.h"
#include "list.h"
#include <Muzzle.h>
#include <stdint.h>

#ifndef MUZOMBIE_DATASET_STRING_POOL_CAPACITY
	#define MUZOMBIE_DATASET_STRING_POOL_CAPACITY 4 KB
#endif

/*

<topic>:<property>=<type><value>.

Example:
foo:bar=u;
graphics:quality=.high_quality;
graphics:fov=i90;
player:name=#Bob;
controls:mouse_sensitivity=f3.5;
debug:show_fps=btrue;

Types
u - undefined
. - enum
i - integer (32-bit)
# - string
f - float (double)
b - boolean

*/

typedef enum : uint8_t
{
	DATASET_VALUE_TYPE_ANY = 0,
	DATASET_VALUE_TYPE_UNDEFINED,
	DATASET_VALUE_TYPE_ENUM,
	DATASET_VALUE_TYPE_INTEGER,
	DATASET_VALUE_TYPE_STRING,
	DATASET_VALUE_TYPE_FLOAT,
	DATASET_VALUE_TYPE_BOOLEAN,
} dataset_value_type;

typedef enum_ordinal (*dataset_enum_mapper_fn)(const char* string);

#define DATASET_VALUE_INIT_UNDEFINED() (dataset_value){.type = DATASET_VALUE_TYPE_ANY}

typedef struct dataset_value
{
	dataset_value_type type;

	union
	{
		int32_t integer;
		enum_ordinal enum_ordinal;
		const char* string;
		double floating;
		mz_boolean boolean;
	};
} dataset_value;

#define DATASET_PROPERTY_INIT_EMPTY(id) (dataset_property){.identifier = id, .expected_type = DATASET_VALUE_TYPE_ANY, .value = DATASET_VALUE_INIT_UNDEFINED()}

typedef struct dataset_property
{
	const char* identifier;
	dataset_enum_mapper_fn enum_mapper;
	dataset_value value;
	dataset_value_type expected_type;
} dataset_property;

#define DATASET_TOPIC_INIT_EMPTY(id) (dataset_topic){.identifier = id, .properties = LIST_INIT_EMPTY(dataset_property)}

typedef struct dataset_topic
{
	LIST(dataset_property) properties;
	const char* identifier;
} dataset_topic;

typedef struct dataset
{
	LIST(dataset_topic) topics;
	mz_boolean strict_mode; // Strict mode means that the parsed final cannot add any topics or properties, only modify already defined topics and properties
} dataset;

mz_boolean parse_dataset_from_file(arena* string_arena, dataset* data, const char* filepath);
void strict_parse_dataset_from_file(arena* string_arena, dataset* data, const char* filepath);
void unload_dataset(dataset* data);

#endif // MUZOMBIE_DATASET_H
