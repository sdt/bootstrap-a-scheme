#include "debug.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

// This will be implemented with setjmp/longjmp soon.
void throw(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    char* msg;
    if (vasprintf(&msg, fmt, ap) < 0) {
        msg = "(message too long)";
    }
    va_end(ap);

    fprintf(stderr, "%s\n", msg);
    free(msg);
    exit(1);
}
