#ifndef INCLUDE_EXCEPTION_H
#define INCLUDE_EXCEPTION_H

#include <setjmp.h>
#include <stdlib.h>

extern jmp_buf*     exception_buf();
extern void         exception_throw(const char* msg, ...);
extern const char*  exception_msg();

#define THROW exception_throw

#endif // INCLUDE_EXCEPTION_H
