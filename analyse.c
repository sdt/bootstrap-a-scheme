#include "analyse.h"

#include "args.h"
#include "executor.h"
#include "types.h"
#include "util.h"
#include "valuestack.h"

#define POP_RET(expr)   Pointer ret = (expr); POP(); return ret;

static Pointer map_analyse(StackIndex pairIndex)
{
    // This should be replaced with a built-in map function.
    if (GET(pairIndex).type != Type_pair) {
        return GET(pairIndex);
    }

    StackIndex carIndex = PUSH(pair_get(GET(pairIndex), 0));
    SET(carIndex, analyse(carIndex));

    StackIndex cdrIndex = PUSH(pair_get(GET(pairIndex), 1));
    SET(cdrIndex, map_analyse(cdrIndex));

    Pointer ret = pair_make(carIndex, cdrIndex);

    DROP(2);

    return ret;
}

static ExecuteHandlerId
analyse_apply(StackIndex opIndex, StackIndex argsIndex, StackIndex valueIndex)
{
    // [ op args ]
    SET(valueIndex, vector_make(2));

    vector_set(valueIndex, 0, analyse(opIndex));
    vector_set(valueIndex, 1, map_analyse(argsIndex));

    return ExecuteHandler_apply;
}

static ExecuteHandlerId
analyse_define(const char* symbol, StackIndex argsIndex, StackIndex valueIndex)
{
    GET_ARGS_EXACTLY(2);
    ARG_CHECKTYPE(0, symbol, "arg 1");

    // [ symbol executor ]
    SET(valueIndex, vector_make(2));

    vector_set(valueIndex, 0, GET(ARG_INDEX(0)));
    vector_set(valueIndex, 1, analyse(ARG_INDEX(1)));

    DROP_ARGS();

    return ExecuteHandler_define;
}

static ExecuteHandlerId
analyse_id(StackIndex exprIndex, StackIndex valueIndex)
{
    // [ value ]
    SET(valueIndex, vector_make(1));
    vector_set(valueIndex, 0, GET(exprIndex));

    return ExecuteHandler_id;
}

static ExecuteHandlerId
analyse_if(const char* symbol, StackIndex argsIndex, StackIndex valueIndex)
{
    GET_ARGS_BETWEEN(2, 3);
    if (ARGS_COUNT() == 2) {
        PUSH(nil_make());   // push nil as the missing 'else' case
    }

    // [ cond then else ]
    SET(valueIndex, vector_make(3));
    for (int i = 0; i < 3; i++) {
        vector_set(valueIndex, i, analyse(ARG_INDEX(i)));
    }

    DROP(3);

    return ExecuteHandler_if;
}

static ExecuteHandlerId
analyse_lambda(const char* symbol, StackIndex argsIndex, StackIndex valueIndex)
{
    GET_ARGS_EXACTLY(2);

    // [ params body ]
    SET(valueIndex, vector_make(2));

    vector_set(valueIndex, 0, GET(ARG_INDEX(0)));
    vector_set(valueIndex, 1, analyse(ARG_INDEX(1)));

    DROP_ARGS();

    return ExecuteHandler_lambda;
}

static ExecuteHandlerId
analyse_var(StackIndex exprIndex, StackIndex valueIndex)
{
    // [ symbol ]
    SET(valueIndex, vector_make(1));
    vector_set(valueIndex, 0, GET(exprIndex));

    return ExecuteHandler_var;
}

static ExecuteHandlerId
analyse_list(StackIndex exprIndex, StackIndex valueIndex)
{
    StackIndex headIndex = PUSH(pair_get(GET(exprIndex), 0));
    StackIndex tailIndex = PUSH(pair_get(GET(exprIndex), 1));

    ExecuteHandlerId handlerId = ExecuteHandler_COUNT;;

    Type type = GET(headIndex).type;
    if (type == Type_symbol) {
        const char* symbol = symbol_get(GET(headIndex));

        if (util_streq(symbol, "define")) {
            handlerId = analyse_define(symbol, tailIndex, valueIndex);
        }
        else if (util_streq(symbol, "if")) {
            handlerId = analyse_if(symbol, tailIndex, valueIndex);
        }
        else if (util_streq(symbol, "lambda")) {
            handlerId = analyse_lambda(symbol, tailIndex, valueIndex);
        }
    }

    if (handlerId == ExecuteHandler_COUNT) {
        handlerId = analyse_apply(headIndex, tailIndex, valueIndex);
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
