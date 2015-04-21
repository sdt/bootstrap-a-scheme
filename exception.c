#include "debug.h"

#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    jmp_buf buf;
    char    msg[1024];
} ExceptionData;

static ExceptionData ex;

void exception_throw(const char* fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(ex.msg, sizeof(ex.msg), fmt, ap);
    va_end(ap);

    longjmp(ex.buf, 1);
}

jmp_buf* exception_buf()
{
    return &ex.buf;
}

const char* exception_msg()
{
    return ex.msg;
}
