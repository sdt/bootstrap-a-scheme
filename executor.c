#include "executor.h"

#include "debug.h"
#include "environment.h"
#include "types.h"
#include "valuestack.h"

typedef Pointer (ExecuteHandler)(StackIndex valueIndex, StackIndex envIndex);

#define HANDLER(ident) \
    static Pointer Handler_##ident(StackIndex valueIndex, StackIndex envIndex)

#define X(ident) HANDLER(ident);
    EXECUTE_HANDLERS_XLIST
#undef X

static ExecuteHandler* handlerTable[] = {
    #define X(ident) Handler_##ident,
        EXECUTE_HANDLERS_XLIST
    #undef X
};

#define ARG(n)  vector_get(GET(valueIndex), (n))

Pointer executor_executeHandler(ExecuteHandlerId handlerId,
                                StackIndex valueIndex, StackIndex envIndex)
{
    ASSERT(handlerId < ExecuteHandler_COUNT,
            "Handler ID %d out of range", handlerId);

    ExecuteHandler* handler = handlerTable[handlerId];
    return handler(valueIndex, envIndex);
}

HANDLER(id)
{
    // [ value ]
    return ARG(0);
}

HANDLER(var)
{
    // [ symbol ]
    return env_get(GET(envIndex), ARG(0));
}

HANDLER(define)
{
    // [ symbol executor ]
    StackIndex valIndex = PUSH(executor_execute(ARG(1), envIndex));
    StackIndex symIndex = PUSH(ARG(0));
    env_set(envIndex, symIndex, valIndex);
    POP();
    return POP(); // value
}
