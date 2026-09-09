#include "strict_alloc.h"
#include <Muzzle.h>

void* strict_alloc(strict_alloc_parameters params)
{
	if (params.bytes == 0)
	{
		if (params.allow_empty)
		{
#ifdef MUZOMBIE_DEBUG_BUILD
			mz_log_status_formatted(LOG_STATUS_INFO, "Created allowed empty allocation for %s.", params.purpose != NULL ? params.purpose : "something");
#endif
			return NULL;
		}

		else
		{
			mz_log_status_formatted(LOG_STATUS_FATAL_ERROR, "Caught unallowed empty allocation for %s.", params.purpose != NULL ? params.purpose : "something");
			__builtin_unreachable();
		}
	}
	
	void* ptr = MZ_MALLOC(params.bytes);

	if (ptr == NULL)
	{
		mz_log_status_formatted(LOG_STATUS_FATAL_ERROR, "Failed to allocate %zu bytes for %s.", params.bytes, params.purpose != NULL ? params.purpose : "something");
		__builtin_unreachable();
	}

#ifdef MUZOMBIE_DEBUG_BUILD
	mz_log_status_formatted(LOG_STATUS_INFO, "Allocated %zu bytes for %s.", params.bytes, params.purpose != NULL ? params.purpose : "something");
#endif

	return ptr;
}
