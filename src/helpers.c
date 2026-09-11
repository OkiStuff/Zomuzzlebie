#include "helpers.h"
#include "strict_alloc.h"
#include <stdio.h>

mz_boolean read_file(const char* filepath, char** out, size_t out_capacity)
{
	FILE* file = NULL;
	
#ifdef _MSC_VER
	fopen_s(&file, filepath, "r");
#else
	file = fopen(filepath, "r");
#endif

	if (file == NULL)
	{
		return MUZZLE_FALSE;
	}
	
	fseek(file, 0, SEEK_END);
	const long int size = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	if (*out == NULL)
	{
		*out = STRICT_ALLOC(sizeof(char) * (size + 1), "a file contents buffer");
		out_capacity = size + 1;
	}

	const long int bytes_to_write = MIN(out_capacity - 1, size);
	fread(*out, bytes_to_write, 1, file);
	(*out)[bytes_to_write] = '\0';
	fclose(file);

	return MUZZLE_TRUE;
}
