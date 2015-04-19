#include "debug.h"

#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

jmp_buf exceptionBuf;
char* exceptionMsg = NULL;

// This will be implemented with setjmp/longjmp soon.
void throw(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    if (vasprintf(&exceptionMsg, fmt, ap) < 0) {
        asprintf(&exceptionMsg, "(exception message too long)");
    }
    va_end(ap);

    longjmp(exceptionBuf, 1);
}
