#include "arena.h"
#include "backend.h"
#include "core/logging.h"
#include "helpers.h"
#include "list.h"
#include "settings.h"
#include "strict_alloc.h"
#include <stdlib.h>
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

	if (data->strict_mode)
	{
		return NULL;
	}
	
	arena_allocation string = arena_alloc(string_arena, sizeof(char) * strlen(identifier) + 1);
	memcpy(string.buffer, identifier, string.length);

	MZ_ASSERT_DETAILED(((char*)(string.buffer))[string.length - 1] == '\0', "Identifier string must be null terminated");

	dataset_topic topic = DATASET_TOPIC_INIT_EMPTY(string.buffer);
	LIST_APPEND(dataset_topic, &data->topics, topic);
	return &LIST_GET_LAST(&data->topics);
}

static inline dataset_property* get_property(arena* string_arena, dataset_topic* topic, const char* identifier, mz_boolean strict_mode)
{
	LIST_FOREACH(&topic->properties, dataset_property, property)
	{
		if (strcmp(property->identifier, identifier) == 0)
		{
			return property;
		}
	}

	if (strict_mode)
	{
		return NULL;
	}

	arena_allocation string = arena_alloc(string_arena, sizeof(char) * strlen(identifier) + 1);
	memcpy(string.buffer, identifier, string.length);

	MZ_ASSERT_DETAILED(((char*)(string.buffer))[string.length - 1] == '\0', "Identifier string must be null terminated");

	dataset_property property = DATASET_PROPERTY_INIT_EMPTY(string.buffer);
	LIST_APPEND(dataset_property, &topic->properties, property);
	return &LIST_GET_LAST(&topic->properties);
}

static inline mz_boolean type_equals(dataset_value_type a, dataset_value_type b)
{
	return (a == DATASET_VALUE_TYPE_ANY || b == DATASET_VALUE_TYPE_ANY)
			? MUZZLE_TRUE
			: a == b;
}

static inline const char* type_to_string(dataset_value_type a)
{
	switch (a)
	{
	case DATASET_VALUE_TYPE_ANY:
		return "any";

	case DATASET_VALUE_TYPE_UNDEFINED:
		return "undefined";

	case DATASET_VALUE_TYPE_ENUM:
		return "enum";

	case DATASET_VALUE_TYPE_INTEGER:
		return "integer";
		
	case DATASET_VALUE_TYPE_STRING:
		return "string";

	case DATASET_VALUE_TYPE_FLOAT:
		return "float";

	case DATASET_VALUE_TYPE_BOOLEAN:
		return "boolean";
	}
}

#define CHECK_IF_SCRATCH_FULL(x) if (scratch_length == scratch_capacity)\
{\
	scratch[scratch_length] = '\0';\
	mz_log_status_formatted(LOG_STATUS_ERROR, "Dataset parsing error: " x " '%s' too large (max length: %zu characters) at %d:%d", scratch, scratch_length, line, character);\
	return MUZZLE_FALSE;\
}

mz_boolean parse_dataset_from_file(arena* string_arena, dataset* data, const char* filepath)
{
	static char scratch[2 KB];
	static const size_t scratch_capacity = (sizeof(scratch) / sizeof(char)) - 1;

	char* file_contents = NULL;

	if (!read_file(filepath, &file_contents, 0))
	{
		mz_log_status_formatted(LOG_STATUS_WARNING, "Failed to open '%s'", filepath);
		return MUZZLE_FALSE;
	}
	
	enum
	{
		START,
		TOPIC,
		PROPERTY,
		TYPE,
		GATHER_LITERAL,
		PARSE_LITERAL
	} parser_state = START;

	dataset_topic* curr_topic = NULL;
	dataset_property* curr_property = NULL;
	size_t scratch_length = 0;

	char* cursor = file_contents;

	uint32_t line = 0;
	uint32_t character = 0;

	LIST(char) string_gathering_buffer = {0};
	
	for (char c = *cursor; c != '\0'; c = *(++cursor))
	{
		if (c == '\n' || c == '\r')
		{
			line++;
			character = 0;
			continue;
		}

		character++;

		switch (parser_state)
		{
		case START:
			parser_state = TOPIC;
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
				break;
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
				curr_property = get_property(string_arena, curr_topic, scratch, data->strict_mode);
				parser_state = TYPE;
				scratch_length = 0;
				break;
			}

			CHECK_IF_SCRATCH_FULL("Property identifier");
			scratch[scratch_length++] = c;
			break;

		case TYPE:
			dataset_value_type type = DATASET_VALUE_TYPE_UNDEFINED;
			
			switch (c)
			{
			case 'u':
				break;

			case '.':
				if (curr_property->enum_mapper == NULL)
				{
					mz_log_status_formatted(LOG_STATUS_ERROR, "Dataset type error: Found value of type enum at %d:%d, but property has no registered enum mapper", line, character);
					return MUZZLE_FALSE;
				}

				type = DATASET_VALUE_TYPE_ENUM;
				break;
			
			case 'i':
				type = DATASET_VALUE_TYPE_INTEGER;
				break;

			case '#':
				type = DATASET_VALUE_TYPE_STRING;
				break;

			case 'f':
				type = DATASET_VALUE_TYPE_FLOAT;
				break;

			case 'b':
				type = DATASET_VALUE_TYPE_BOOLEAN;
				break;

			default:
				mz_log_status_formatted(LOG_STATUS_ERROR, "Dataset type error: Unknown value type '%c', at %d:%d", c, line, character);
				return MUZZLE_FALSE;
			}
			
			if (!type_equals(curr_property->expected_type, type))
			{
				mz_log_status_formatted(LOG_STATUS_ERROR, "Dataset type error: Expected value of type %s, got value of type %s at %d:%d", type_to_string(curr_property->expected_type), type_to_string(type), line, character);
				return MUZZLE_FALSE;
			}

			curr_property->value.type = type;
			parser_state = GATHER_LITERAL;
			
			break;

		case GATHER_LITERAL:
			switch (curr_property->value.type)
			{
			case DATASET_VALUE_TYPE_ANY:
#ifdef MUZOMBIE_DEBUG_BUILD
				mz_log_status_formatted(LOG_STATUS_FATAL_ERROR, "Dataset parsing error: corrupted state, value type is any");
#endif
				__builtin_unreachable();
				break;
			
			case DATASET_VALUE_TYPE_UNDEFINED:
				if (c != ';')
				{
					mz_log_status_formatted(LOG_STATUS_ERROR, "Dataset parsing error: Expected ';', got '%c' at %d:%d\n\tHint: value is of type undefined, undefined values are written as <topic>:<property>=u;", c, line, character);
					return MUZZLE_FALSE;
				}

				break;

			case DATASET_VALUE_TYPE_ENUM:
			case DATASET_VALUE_TYPE_INTEGER:
			case DATASET_VALUE_TYPE_FLOAT:
			case DATASET_VALUE_TYPE_BOOLEAN:
				if (c == ';')
				{
					if (scratch_length == 0)
					{
						mz_log_status_formatted(LOG_STATUS_ERROR, "Dataset parsing error: Expected value, got ';' at %d:%d", line, character);
						return MUZZLE_FALSE;
					}

					scratch[scratch_length] = '\0';
					parser_state = PARSE_LITERAL;
					break;
				}

				CHECK_IF_SCRATCH_FULL("Literal");
				scratch[scratch_length++] = c;
				break;

			case DATASET_VALUE_TYPE_STRING:
				if (c == ';')
				{
					if (scratch_length == 0)
					{
						mz_log_status_formatted(LOG_STATUS_ERROR, "Dataset parsing error: Expected value, got ';' at %d:%d", line, character);
						return MUZZLE_FALSE;
					}

					if (string_gathering_buffer.data == NULL)
					{
						scratch[scratch_length] = '\0';
					}
 
					else
					{
						LIST_APPEND(char, &string_gathering_buffer, '\0');
					}
					
					parser_state = PARSE_LITERAL;
					cursor--;
					break;
				}

				if (scratch_length == scratch_capacity)
				{
					LIST(char) new_list = LIST_INIT_WITH_CAPACITY(char, (scratch_length + 128));
					memcpy(new_list.data, scratch, sizeof(char) * scratch_length);
					
					string_gathering_buffer.data = new_list.data;
					string_gathering_buffer.length = new_list.length;
					string_gathering_buffer.capacity = new_list.capacity;
				}

				if (string_gathering_buffer.data == NULL)
				{
					scratch[scratch_length++] = c;
				}

				else
				{
					LIST_APPEND(char, &string_gathering_buffer, c);
					scratch_length++;
				}
				
				break;
			}

			break;

		case PARSE_LITERAL:
			MZ_ASSERT_DETAILED(c == ';', "Current character should be ';'");
			
			switch (curr_property->value.type)
			{
			case DATASET_VALUE_TYPE_ANY:
#ifdef MUZOMBIE_DEBUG_BUILD
				mz_log_status_formatted(LOG_STATUS_FATAL_ERROR, "Dataset parsing error: corrupted state, value type is any");
#endif
				__builtin_unreachable();
				break;

			case DATASET_VALUE_TYPE_UNDEFINED:
				break;

			case DATASET_VALUE_TYPE_ENUM:
				mz_boolean failed = MUZZLE_FALSE;
				enum_ordinal ordinal = curr_property->enum_mapper(scratch, &failed);

				if (failed)
				{
					mz_log_status_formatted(LOG_STATUS_ERROR, "Dataset parsing error: Invalid enum '%s' at %d:%d", scratch, line, character);
					return MUZZLE_FALSE;
				}

				curr_property->value.enum_ordinal = ordinal;
				break;

			case DATASET_VALUE_TYPE_INTEGER:
				curr_property->value.integer = atoi(scratch);
				break;

			case DATASET_VALUE_TYPE_STRING:
				arena_allocation string = arena_alloc(string_arena, sizeof(char) * (scratch_length + 1));
				char* src = string_gathering_buffer.data != NULL ? string_gathering_buffer.data : scratch;

				memcpy(string.buffer, src, sizeof(char) * scratch_length);
				((char*)(string.buffer))[scratch_length] = '\0';

				if (string_gathering_buffer.data != NULL)
				{
					UNLOAD_LIST(&string_gathering_buffer);
				}
				
				break;

			case DATASET_VALUE_TYPE_FLOAT:
				curr_property->value.floating = atof(scratch);
				break;

			case DATASET_VALUE_TYPE_BOOLEAN:
				if (strcmp(scratch, "true") == 0)
				{
					curr_property->value.boolean = MUZZLE_TRUE;
				}

				else if (strcmp(scratch, "false") == 0)
				{
					curr_property->value.boolean = MUZZLE_FALSE;
				}

				else
				{
					mz_log_status_formatted(LOG_STATUS_ERROR, "Dataset parsing error: Expected boolean value of 'true' or 'false', found invalid boolean value '%s' at %d:%d", scratch, line, character);
					return MUZZLE_FALSE;
				}
				
				break;
			}

			memset(&string_gathering_buffer, 0, sizeof(string_gathering_buffer));
			scratch_length = 0;
			curr_topic = NULL;
			curr_property = NULL;
			parser_state = START;
			
			break;
		}
	}

	if (parser_state != START)
	{
		const char* parser_state_str;

		switch (parser_state)
		{
		case START: __builtin_unreachable(); break;
		case TOPIC: parser_state_str = "topic"; break;
		case PROPERTY: parser_state_str = "property"; break;
		case TYPE: parser_state_str = "type"; break;
			
		case GATHER_LITERAL:
		case PARSE_LITERAL:
			parser_state_str = "literal";
			break;
		}
			
		mz_log_status_formatted(LOG_STATUS_ERROR, "Dataset parsing error: Expected %s, found EOF at %d:%d", parser_state_str, line, character);
		return MUZZLE_FALSE;
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
