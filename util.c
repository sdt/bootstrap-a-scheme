#ifndef INCLUDE_UTIL_C
#define INCLUDE_UTIL_C

#include "util.h"

#include <string.h>

int util_streq(const char* a, const char* b)
{
    return strcmp(a, b) == 0;
}

#endif // INCLUDE_UTIL_C
