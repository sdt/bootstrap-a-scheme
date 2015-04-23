#include "allocator.h"
#include "args.h"
#include "debug.h"
#include "core.h"
#include "environment.h"
#include "exception.h"
#include "gc.h"
#include "input.h"
#include "reader.h"
#include "symtab.h"
#include "types.h"
#include "valuestack.h"

#include <stdio.h>
#include <string.h>

static Pointer eval_list(StackIndex listIndex, StackIndex envIndex);
Pointer eval(StackIndex astIndex, StackIndex envIndex);

int main(int argc, char* argv[])
{
    int heapSize = 16 * 1024 * 1024;

    allocator_init(heapSize);
    valuestack_init(heapSize / 4);
    env_init();
    symtab_init();
    core_init();
    input_init();
    gc_run();

    StackIndex prevTop = valuestack_top();

    char* input;
    while ((input = input_get("bas> ")) != NULL) {
        StackIndex top = valuestack_top();
        ASSERT(top == prevTop, "Value stack out of whack! Was %d, is %d",
            prevTop, top);
        prevTop = top;

        if (setjmp(*exception_buf()) != 0) {
            printf("%s\n", exception_msg());
            valuestack_popTo(top);
            continue;
        }

        StackIndex astIndex = PUSH(readLine(input));
        StackIndex envIndex = PUSH(env_root());

        print(eval(astIndex, envIndex));

        valuestack_drop(2);
    }

    return 0;
}

static Pointer _eval(StackIndex astIndex, StackIndex envInIndex)
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

#define WHEN(str) if (strcmp(symbol, str) == 0)

            WHEN("define") {
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

            WHEN("if") {
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

            WHEN("lambda") {
                // (lambda params body)
                GET_ARGS_EXACTLY(2);
                StackIndex paramsIndex = ARG_INDEX(0);
                StackIndex bodyIndex   = ARG_INDEX(1);

                DROP_RET(lambda_make(paramsIndex, bodyIndex, envIndex));
            }

#undef WHEN
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

Pointer eval(StackIndex astIndex, StackIndex envIndex)
{
    // Rather than manage the stack inside eval, handle it all here.
    int top = valuestack_top();
    Pointer ret = _eval(astIndex, envIndex);
    valuestack_popTo(top);
    return ret;
}

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
