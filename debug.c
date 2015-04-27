#include "debug.h"

#include <execinfo.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#define STACKTRACE_SIZE 16

void
stacktrace(int skip)
{
    void *array[STACKTRACE_SIZE];
    size_t size;
    char **strings;
    size_t i;

    size = backtrace(array, STACKTRACE_SIZE);
    strings = backtrace_symbols(array, size);

    for (i = skip + 1; i < size; i++) {
        printf("%s\n", strings[i]);
    }
    free(strings);
}

void assert_verbose(int cond, const char* file, int line, const char* fmt, ...)
{
    if (cond) {
        return;
    }

    fprintf(stderr, "Internal error %s(%d): ", file, line);

    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    fprintf(stderr, "\n");

    stacktrace(1);

    exit(1);
}
