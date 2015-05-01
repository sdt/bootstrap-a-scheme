#include "eval.h"
#include "config.h"

#include "analyse.h"
#include "args.h"
#include "environment.h"
#include "exception.h"
#include "executor.h"
#include "types.h"
#include "util.h"
#include "valuestack.h"

static Pointer eval_internal(StackIndex astIndex, StackIndex envInIndex);

Pointer eval(StackIndex astIndex, StackIndex envIndex)
{
    // Rather than manage the stack inside eval, handle it all here.
    int top = valuestack_top();
    Pointer ret = eval_internal(astIndex, envIndex);
    valuestack_popTo(top);
    return ret;
}

#if EVAL_METHOD == EVAL_INTERPRET

static Pointer eval_list(StackIndex listIndex, StackIndex envIndex)
{
    if (GET(listIndex).type == Type_nil) {
        return GET(listIndex);
    }

    StackIndex carIndex = PUSH(PAIR_GET(listIndex, 0));
    StackIndex cdrIndex = PUSH(PAIR_GET(listIndex, 1));

    SET(carIndex, eval(carIndex, envIndex));
    SET(cdrIndex, eval_list(cdrIndex, envIndex));

    Pointer ret = pair_make(carIndex, cdrIndex);

    DROP(2);

    return ret;
}

static Pointer eval_internal(StackIndex astIndex, StackIndex envInIndex)
{
    StackIndex opIndex   = valuestack_reserve();
    StackIndex argsIndex = valuestack_reserve();
    StackIndex envIndex  = PUSH(GET(envInIndex));

    const char* symbol;

    while (1) {
        Type astType = GET(astIndex).type;

        if (astType == Type_symbol)
            return env_get(GET(envIndex), GET(astIndex));

        if (astType != Type_pair)
            return GET(astIndex);

        SET(opIndex, PAIR_GET(astIndex, 0));

        if (GET(opIndex).type == Type_symbol) {
            SET(argsIndex, PAIR_GET(astIndex, 1));
            symbol = symbol_get(GET(opIndex));

            if (util_streq(symbol, "define")) {
                // (define symbol expr)
                // (define (fn params) body)
                //      -> (define fn (lambda params body))
                GET_ARGS_EXACTLY(2);
                ARG_CHECKTYPE(0, symbol, "arg 1");
                StackIndex symIndex = ARG_INDEX(0);
                StackIndex valIndex = ARG_INDEX(1);

                // Evaluate the expression
                SET(valIndex, eval(valIndex, envIndex));

                // Install the result into the environment.
                env_set(envIndex, symIndex, valIndex);

                DROP_RET(GET(valIndex));
            }

            if (util_streq(symbol, "if")) {
                // (if cond then else?)
                GET_ARGS_BETWEEN(2, 3);
                if (ARGS_COUNT() == 2) {
                    PUSH(nil_make()); // insert a nil else if not provided
                }

                Pointer cond = eval(ARG_INDEX(0), envIndex);
                SET(astIndex, GET(ARG_INDEX(pointer_isTrue(cond) ? 1 : 2)));

                DROP_ARGS();
                continue; // TCO
            }

            if (util_streq(symbol, "lambda")) {
                // (lambda params body)
                GET_ARGS_EXACTLY(2);
                StackIndex paramsIndex = ARG_INDEX(0);
                StackIndex bodyIndex   = ARG_INDEX(1);

                DROP_RET(lambda_make(paramsIndex, bodyIndex, envIndex));
            }
        }

        // If we get here, no special forms applied.
        SET(astIndex, eval_list(astIndex, envIndex));

        SET(opIndex,   PAIR_GET(astIndex, 0));
        SET(argsIndex, PAIR_GET(astIndex, 1));

        switch (GET(opIndex).type) {
            case Type_builtin:
                return builtin_apply(GET(opIndex), argsIndex, envIndex);

            case Type_lambda:
                SET(envIndex, lambda_prepareEnv(opIndex, argsIndex));
                SET(astIndex, lambda_getBody(GET(opIndex)));
                continue; //TCO

            default:
                THROW("%s is not applicable", type_name(GET(opIndex).type));
                break;
        }
    }
}

#elif EVAL_METHOD == EVAL_ANALYSE

static Pointer eval_internal(StackIndex astIndex, StackIndex envIndex)
{
    Pointer analysed = analyse(astIndex);
    Pointer evaluated = executor_execute(analysed, envIndex);
    return evaluated;
}

#else

#error Unknown evaluation method

#endif
