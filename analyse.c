#include "analyse.h"

#include "args.h"
#include "executor.h"
#include "types.h"
#include "util.h"
#include "valuestack.h"

#define POP_RET(expr)   Pointer ret = (expr); POP(); return ret;

static ExecuteHandlerId
analyse_define(StackIndex argsIndex, StackIndex valueIndex)
{
    const char* symbol = "define";

    GET_ARGS_EXACTLY(2);
    ARG_CHECKTYPE(0, symbol, "arg 1");

    // [ symbol executor ]
    SET(valueIndex, vector_make(2));

    vector_set(GET(valueIndex), 0, GET(ARG_INDEX(0)));
    vector_set(GET(valueIndex), 1, analyse(ARG_INDEX(1)));

    DROP_ARGS();

    return ExecuteHandler_define;
}

static ExecuteHandlerId
analyse_id(StackIndex exprIndex, StackIndex valueIndex)
{
    // [ value ]
    SET(valueIndex, vector_make(1));
    vector_set(GET(valueIndex), 0, GET(exprIndex));

    return ExecuteHandler_id;
}

static ExecuteHandlerId
analyse_var(StackIndex exprIndex, StackIndex valueIndex)
{
    // [ symbol ]
    SET(valueIndex, vector_make(1));
    vector_set(GET(valueIndex), 0, GET(exprIndex));

    return ExecuteHandler_var;
}

static ExecuteHandlerId
analyse_list(StackIndex exprIndex, StackIndex valueIndex)
{
    StackIndex headIndex = PUSH(pair_get(GET(exprIndex), 0));
    StackIndex tailIndex = PUSH(pair_get(GET(exprIndex), 1));

    ExecuteHandlerId handlerId;

    Type type = GET(headIndex).type;
    if (type == Type_symbol) {
        const char* symbol = symbol_get(GET(headIndex));

        if (util_streq(symbol, "define")) {
            handlerId = analyse_define(tailIndex, valueIndex);
        }
        else {
            // Default case just return the value.
            handlerId = analyse_id(exprIndex, valueIndex);
        }
    }
    else {
        // Default case just return the value.
        handlerId = analyse_id(exprIndex, valueIndex);
    }

    DROP(2);
    return handlerId;
}

Pointer analyse(StackIndex exprIndex)
{
    StackIndex valueIndex = RESERVE();

    ExecuteHandlerId handlerId;
    Type type = GET(exprIndex).type;

    if (type == Type_symbol) {
        handlerId = analyse_var(exprIndex, valueIndex);
    }
    else if (type != Type_pair) {
        handlerId = analyse_id(exprIndex, valueIndex);
    }
    else {
        handlerId = analyse_list(exprIndex, valueIndex);
    }

    Pointer ret = executor_make(handlerId, valueIndex);
    DROP(1);
    return ret;
}
