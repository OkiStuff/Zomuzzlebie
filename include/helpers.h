#ifndef MUZOMBIE_HELPERS_H
#define MUZOMBIE_HELPERS_H

#include <Muzzle.h>

#define KB * 1024L
#define MB * 1024 KB
#define GB * 1024 MB

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

mz_boolean read_file(const char* filepath, char** out, size_t out_capacity);

#endif // MUZOMBIE_HELPERS_H
