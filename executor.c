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
    return vector_get(GET(valueIndex), 0);
}

HANDLER(var)
{
    return env_get(GET(envIndex), vector_get(GET(valueIndex), 0));
}
