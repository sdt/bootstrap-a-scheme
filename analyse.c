#include "analyse.h"

#include "args.h"
#include "executor.h"
#include "types.h"
#include "valuestack.h"

#define POP_RET(expr)   Pointer ret = (expr); POP(); return ret;

static ExecuteHandlerId analyse_id(StackIndex exprIndex, StackIndex valueIndex)
{
    SET(valueIndex, vector_make(1));
    vector_set(GET(valueIndex), 0, GET(exprIndex));

    return ExecuteHandler_id;
}

static ExecuteHandlerId analyse_var(StackIndex exprIndex, StackIndex valueIndex)
{
    SET(valueIndex, vector_make(1));
    vector_set(GET(valueIndex), 0, GET(exprIndex));

    return ExecuteHandler_var;
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
        // Default case just return the value.
        handlerId = analyse_id(exprIndex, valueIndex);
    }

    Pointer ret = executor_make(handlerId, valueIndex);
    DROP(1);
    return ret;
}
