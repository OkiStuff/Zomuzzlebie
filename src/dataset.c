#include "core/logging.h"
#include "helpers.h"
#include "settings.h"
#include "strict_alloc.h"

mz_boolean parse_dataset_from_file(arena* string_arena, dataset* data, const char* filepath)
{
	char* file_contents = NULL;

	if (!read_file(filepath, &file_contents, 0))
	{
		mz_log_status_formatted(LOG_STATUS_WARNING, "Failed to open '%s'", filepath);
		return MUZZLE_FALSE;
	}

	
	enum
	{
		TOPIC,
		COLON,
		PROPERTY,
		EQUALS,
		TYPE,
		LITERAL
	} parser_state = TOPIC;

	arena_allocation scratch = {0};
	char* scratch_cursor = NULL;
	char* cursor = file_contents;
	
	for (char c = *cursor; c != '\0'; c = *(++cursor))
	{
		if (c == '\n' || c == '\r')
		{
			parser_state = TOPIC;
			continue;
		}

		switch (parser_state)
		{
		case TOPIC:
			if (scratch.buffer == NULL)
			{
				scratch = arena_alloc(string_arena, sizeof(char) * 64);
				scratch_cursor = scratch.buffer;
				*scratch_cursor = '\0';
			}

			if (c == ':')
			{
				if (*scratch_cursor == '\0')
				{
					// error
				}

				else
				{
					size_t excess = (scratch.buffer + scratch.length) - (void*)(scratch_cursor);

					if (excess > 0)
					{
						arena_release(string_arena, excess);
					}

					arena_allocation new = arena_alloc(string_arena, 1);
					MZ_ASSERT_DETAILED(new.buffer == scratch.buffer + (scratch.length - excess), "New buffer must originate from old buffer");
					scratch.length = new.length;

					((char*)(scratch.buffer))[scratch.length-1] = '\0';

					
				}
			}
			
			*scratch_cursor++ = c;

			if (scratch_cursor > (char*)(scratch.buffer + (scratch.length * sizeof(char))))
			{
				arena_allocation new = arena_alloc(string_arena, 64);
				MZ_ASSERT_DETAILED(new.buffer == scratch.buffer + scratch.length, "New buffer must originate from old buffer");
				scratch.length = new.length;
			}
		}
	}

	MZ_FREE(file_contents);

	return MUZZLE_TRUE;
}

void strict_parse_dataset_from_file(arena* string_arena, dataset* data, const char* filepath)
{
	if (!parse_dataset_from_file(string_arena, data, filepath))
	{
		mz_log_status_formatted(LOG_STATUS_FATAL_ERROR, "Failed to read dataset '%s'", filepath);
		__builtin_unreachable();
	}
}

void unload_dataset(dataset* data);
