#ifndef INCLUDE_CORE_H
#define INCLUDE_CORE_H

#include "types.h"

#define CORE_HANDLERS_XLIST \
    X("cons",       cons)           \
    X("car",        car)            \
    X("cdr",        cdr)            \
    X("empty?",     isEmpty)        \
    X("+",          add)

/*
    X("-", sub) \
    X("*", mul) \
    X("/", div) \
    X("car", car) \
    X("cdr", cdr) \
    X("=", equals)
*/

typedef enum {
    #define X(sym, ident) Core_##ident,
        CORE_HANDLERS_XLIST
    #undef X

    Core_COUNT
} CoreHandlerId;

extern void     core_init();
extern Pointer  core_apply(CoreHandlerId id, Pointer args, Pointer env);

#endif // INCLUDE_CORE_H
