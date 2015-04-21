#include "allocator.h"
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
    int heapSize = 1 * 1024;

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
            fprintf(stderr, "%s\n", exception_msg());
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

static Pointer _eval(StackIndex astIndex, StackIndex envIndex)
{
    StackIndex opIndex   = valuestack_reserve();
    StackIndex argsIndex = valuestack_reserve();

    const char* sym;

    while (1) {
        Type astType = GET(astIndex).type;

        if (astType == Type_symbol)
            return env_get(GET(envIndex), GET(astIndex));

        if (astType != Type_pair)
            return GET(astIndex);

        SET(opIndex, PAIR_GET(astIndex, 0));

        if (GET(opIndex).type == Type_symbol) {
            SET(argsIndex, PAIR_GET(astIndex, 1));
            sym = symbol_get(GET(opIndex));

            if (strcmp(sym, "define") == 0) {
                StackIndex symIndex = PUSH(NTH(argsIndex, 0));
                StackIndex valIndex = PUSH(NTH(argsIndex, 1));
                SET(valIndex, eval(valIndex, envIndex));
                env_set(envIndex, symIndex, valIndex);
                return GET(valIndex);
            }

            if (strcmp(sym, "if") == 0) {
                StackIndex condIndex = PUSH(NTH(argsIndex, 0));
                // TODO: handle the no-else case
                int which = pointer_isTrue(eval(condIndex, envIndex)) ? 1 : 2;
                POP(); // condIndex

                SET(astIndex, NTH(argsIndex, which));
                continue; // TCO
            }

            if (strcmp(sym, "lambda") == 0) {
                StackIndex paramsIndex = PUSH(NTH(argsIndex, 0));
                StackIndex bodyIndex   = PUSH(NTH(argsIndex, 1));
                return lambda_make(paramsIndex, bodyIndex, envIndex);
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
                SET(envIndex, lambda_prepareEnv(opIndex, argsIndex, envIndex));
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
