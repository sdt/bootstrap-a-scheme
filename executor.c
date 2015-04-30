#include "executor.h"

#include "analyse.h"
#include "debug.h"
#include "environment.h"
#include "exception.h"
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

static Pointer map_execute(StackIndex pairIndex, StackIndex envIndex)
{
    // This should be replaced with a built-in map function.
    if (GET(pairIndex).type != Type_pair) {
        return GET(pairIndex);
    }

    StackIndex carIndex = PUSH(pair_get(GET(pairIndex), 0));
    SET(carIndex, executor_execute(GET(carIndex), envIndex));

    StackIndex cdrIndex = PUSH(pair_get(GET(pairIndex), 1));
    SET(cdrIndex, map_execute(cdrIndex, envIndex));

    Pointer ret = pair_make(carIndex, cdrIndex);

    DROP(2);

    return ret;
}

HANDLER(apply)
{
    // [ op args ]
    //StackIndex opIndex   = PUSH(executor_execute(ARG(0), envIndex));
    StackIndex opIndex   = PUSH(ARG(0));
    StackIndex argsIndex = PUSH(ARG(1));

    SET(opIndex, executor_execute(GET(opIndex), envIndex));
    SET(argsIndex, map_execute(argsIndex, envIndex));

    Pointer ret;

    switch (GET(opIndex).type) {
        case Type_builtin:
            ret = builtin_apply(GET(opIndex), argsIndex, envIndex);
            break;

        case Type_lambda:
            SET(envIndex, lambda_prepareEnv(opIndex, argsIndex));
            Pointer body = lambda_getBody(GET(opIndex));
            ret = executor_execute(body, envIndex);
            break;

        default:
            THROW("%s is not applicable", type_name(GET(opIndex).type));
            break;
    }

    DROP(2);
    return ret;
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

HANDLER(id)
{
    // [ value ]
    return ARG(0);
}

HANDLER(if)
{
    // [ cond then else ]
    Pointer cond = executor_execute(ARG(0), envIndex);
    int resultArg = pointer_isTrue(cond) ? 1 : 2;
    return executor_execute(ARG(resultArg), envIndex);
}

HANDLER(lambda)
{
    // [ params body ]
    StackIndex paramsIndex = PUSH(ARG(0));
    StackIndex bodyIndex   = PUSH(ARG(1));
    Pointer ret = lambda_make(paramsIndex, bodyIndex, envIndex);
    DROP(2);
    return ret;
}

HANDLER(var)
{
    // [ symbol ]
    return env_get(GET(envIndex), ARG(0));
}
