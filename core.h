#ifndef INCLUDE_CORE_H
#define INCLUDE_CORE_H

#include "valuestack.h"

#define CORE_HANDLERS_XLIST \
    X("cons",       cons)           \
    X("car",        car)            \
    X("cdr",        cdr)            \
    X("*dump*",     dump)           \
    X("empty?",     isEmpty)        \
    X("*gc*",       gc)             \
    X("vector",     vector)         \
    X("+",          add)            \
    X("-",          sub)            \
    X("<",          lt)             \
    X("*",          mul)            \
    X("=",          equals)

typedef enum {
    #define X(sym, ident) Core_##ident,
        CORE_HANDLERS_XLIST
    #undef X

    Core_COUNT
} CoreHandlerId;

extern void     core_init();
extern Pointer  core_apply(CoreHandlerId id,
                           StackIndex argsIndex, StackIndex envIndex);

#endif // INCLUDE_CORE_H
