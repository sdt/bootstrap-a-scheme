#include "debug.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void assert_verbose(int cond, const char* file, int line, const char* fmt, ...)
{
    if (cond) {
        return;
    }

    fprintf(stderr, "Assertion failed %s(%d): ", file, line);

    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    fprintf(stderr, "\n");

    exit(1);
}
