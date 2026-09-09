#include "arena.h"
#include "backend.h"
#include "core/logging.h"
#include "helpers.h"
#include "list.h"
#include "settings.h"
#include "strict_alloc.h"
#include <string.h>

static inline dataset_topic* get_topic(arena* string_arena, dataset* data, const char* identifier)
{
	LIST_FOREACH(&data->topics, dataset_topic, topic)
	{
		if (strcmp(topic->identifier, identifier) == 0)
		{
			return topic;
		}
	}

	arena_allocation string = arena_alloc(string_arena, sizeof(char) * strlen(identifier) + 1);
	memcpy(string.buffer, identifier, string.length);

	MZ_ASSERT_DETAILED(((char*)(string.buffer))[string.length - 1] == '\0', "Identifier string must be null terminated");

	dataset_topic topic = DATASET_TOPIC_INIT_EMPTY(string.buffer);
	LIST_APPEND(dataset_topic, &data->topics, topic);
	return &LIST_GET_LAST(&data->topics);
}

static inline dataset_property* get_property(arena* string_arena, dataset_topic* topic, const char* identifier)
{
	LIST_FOREACH(&topic->properties, dataset_property, property)
	{
		if (strcmp(property->identifier, identifier) == 0)
		{
			return property;
		}
	}

	arena_allocation string = arena_alloc(string_arena, sizeof(char) * strlen(identifier) + 1);
	memcpy(string.buffer, identifier, string.length);

	MZ_ASSERT_DETAILED(((char*)(string.buffer))[string.length - 1] == '\0', "Identifier string must be null terminated");

	dataset_property property = DATASET_PROPERTY_INIT_EMPTY(string.buffer);
	LIST_APPEND(dataset_property, &topic->properties, property);
	return &LIST_GET_LAST(&topic->properties);
}

#define CHECK_IF_SCRATCH_FULL(x) if (scratch_length == sizeof(scratch) / sizeof(char) - 1)\
{\
	scratch[scratch_length] = '\0';\
	mz_log_status_formatted(LOG_STATUS_ERROR, "Dataset parsing error: " x " '%s' too large (max length: %zu characters) at %d:%d", scratch, scratch_length, line, character);\
	return MUZZLE_FALSE;\
}

mz_boolean parse_dataset_from_file(arena* string_arena, dataset* data, const char* filepath)
{
	static char scratch[2 KB];

	char* file_contents = NULL;

	if (!read_file(filepath, &file_contents, 0))
	{
		mz_log_status_formatted(LOG_STATUS_WARNING, "Failed to open '%s'", filepath);
		return MUZZLE_FALSE;
	}
	
	enum
	{
		TOPIC,
		PROPERTY,
		TYPE,
		LITERAL
	} parser_state = TOPIC;

	dataset_topic* curr_topic = NULL;
	size_t scratch_length = 0;

	char* cursor = file_contents;

	uint32_t line = 0;
	uint32_t character = 0;
	
	for (char c = *cursor; c != '\0'; c = *(++cursor))
	{
		if (c == '\n' || c == '\r')
		{
			line++;
			character = 0;
			scratch_length = 0;
			curr_topic = NULL;
			parser_state = TOPIC;
			continue;
		}

		character++;

		switch (parser_state)
		{
		case TOPIC:
			if (c == ':')
			{
				if (scratch_length == 0)
				{
					mz_log_status_formatted(LOG_STATUS_ERROR, "Dataset parsing error: Expected <topic>, got ':' at %d:%d", line, character);
					return MUZZLE_FALSE;
				}

				scratch[scratch_length] = '\0';
				curr_topic = get_topic(string_arena, data, scratch);
				parser_state = PROPERTY;
				scratch_length = 0;
			}

			CHECK_IF_SCRATCH_FULL("Topic identifier");
			scratch[scratch_length++] = c;
			break;

		case PROPERTY:
			if (c == '=')
			{
				if (scratch_length == 0)
				{
					mz_log_status_formatted(LOG_STATUS_ERROR, "Dataset parsing error: Expected <property>, got '=' at %d:%d", line, character);
					return MUZZLE_FALSE;
				}

				scratch[scratch_length] = '\0';
				curr_property = get_property(string_arena, data, scratch);
				parser_state = TYPE;
			}
			break;
		}
	}

	MZ_FREE(file_contents);

	return MUZZLE_TRUE;
}

#undef CHECK_IF_SCRATCH_FULL

void strict_parse_dataset_from_file(arena* string_arena, dataset* data, const char* filepath)
{
	if (!parse_dataset_from_file(string_arena, data, filepath))
	{
		mz_log_status_formatted(LOG_STATUS_FATAL_ERROR, "Failed to read dataset '%s'", filepath);
		__builtin_unreachable();
	}
}

void unload_dataset(dataset* data);
