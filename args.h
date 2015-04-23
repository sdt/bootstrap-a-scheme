#ifndef INCLUDE_ARGS_H
#define INCLUDE_ARGS_H

#include "types.h"

extern int        args_count(Pointer args);
extern StackIndex args_extract(const char* caller, Pointer args,
                               int min, int max);
extern Pointer    args_checkType(const char* caller, const char* argName,
                                 Pointer argPtr, Type expected);

// These macros assume the following:
//      StackIndex argsIndex
//      const char* symbol
#define GET_ARGS_BETWEEN(min, max) \
    StackIndex localsIndex = args_extract(symbol, GET(argsIndex), min, max); \
    int localsCount = valuestack_top() - localsIndex;
#define GET_ARGS_EXACTLY(count) GET_ARGS_BETWEEN(count, count)
#define ARGS_COUNT()    (localsCount)
#define ARG_INDEX(n)    (localsIndex+(n))
#define ARG_CHECKTYPE(n, type, name) \
    args_checkType(symbol, name, GET(ARG_INDEX(n)), Type_##type)
#define DROP_ARGS()     valuestack_drop(localsCount)

#endif // INCLUDE_ARGS_H
