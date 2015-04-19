#ifndef INCLUDE_EXCEPTION_H
#define INCLUDE_EXCEPTION_H

#include <setjmp.h>
#include <stdlib.h>

extern jmp_buf exceptionBuf;
extern char* exceptionMsg;
extern void throw(const char* msg, ...);

#define EXCEPTION_SCOPE \
    if (setjmp(exceptionBuf) != 0) { \
        if (exceptionMsg == NULL) { \
            fprintf(stderr, "(unexpected exception)\n"); \
        } \
        else {\
            fprintf(stderr, "%s\n", exceptionMsg); \
            free(exceptionMsg); \
        } \
        continue; \
    } else { }

#endif // INCLUDE_EXCEPTION_H
