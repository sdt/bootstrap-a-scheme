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
    valuestack_init(heapSize / 16);
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

        StackIndex astIndex = valuestack_push(readLine(input));
        print(eval(astIndex, env_root()));

        valuestack_drop(1);
    }

    return 0;
}

#define GET(stackIndex)                 valuestack_get(stackIndex)
#define SET(stackIndex, ptr)            valuestack_set(stackIndex, ptr)
#define PAIR_GET(stackIndex, which)     pair_get(GET(stackIndex), which)
#define PUSH(ptr)                       valuestack_push(ptr)
#define NTH(stackIndex, n)              nth(GET(stackIndex), n)

static Pointer nth(Pointer ptr, int n)
{
    for (int i = 0; i < n; i++) {
        ptr = pair_get(ptr, 1);
    }
    return pair_get(ptr, 0);
}

static Pointer _eval(StackIndex astIndex, StackIndex envIndex)
{
    StackIndex opIndex   = valuestack_reserve();
    StackIndex argsIndex = valuestack_reserve();

    const char* sym;

    switch (GET(astIndex).type) {
    default:
        return GET(astIndex);

    case Type_symbol:
        return env_get(GET(envIndex), GET(astIndex));

    case Type_pair:
        SET(opIndex, PAIR_GET(astIndex, 0));

        if (GET(opIndex).type == Type_symbol) {
            SET(argsIndex, PAIR_GET(astIndex, 1));
            sym = symbol_get(GET(opIndex));

            if (strcmp(sym, "define") == 0) {
                StackIndex symIndex = PUSH(NTH(argsIndex, 0));
                StackIndex valIndex = PUSH(NTH(argsIndex, 1));
                SET(valIndex, eval(valIndex, envIndex));
                env_set(GET(envIndex), GET(symIndex), GET(valIndex));
                return GET(valIndex);
            }

            if (strcmp(sym, "if") == 0) {
                StackIndex condIndex = PUSH(NTH(argsIndex, 0));
                int which = pointer_isTrue(eval(condIndex, envIndex)) ? 1 : 2;
                StackIndex retIndex = PUSH(NTH(argsIndex, which));

                return eval(retIndex, envIndex);
            }

            /*
            if (strcmp(sym, "lambda") == 0) {
                Pointer params = pair_get(args, 0); args = pair_get(args, 1);
                Pointer body   = pair_get(args, 0); args = pair_get(args, 1);
                return lambda_make(params, body, env);
            }
            */
        }

        // If we get here, no special forms applied.
        SET(astIndex, eval_list(astIndex, envIndex));

        SET(opIndex,   PAIR_GET(astIndex, 0));
        SET(argsIndex, PAIR_GET(astIndex, 1));

        switch (GET(opIndex).type) {
            case Type_builtin:
                return builtin_apply(GET(opIndex), argsIndex, envIndex);

/*
            case Type_lambda:
                return lambda_apply(opIndex, argsIndex, envIndex);
*/

            default:
                THROW("%s is not applicable", type_name(GET(opIndex).type));
                break;
        }

        // We don't actually get here.
        return GET(astIndex);
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

    Pointer ret = pair_make(GET(carIndex), GET(cdrIndex));

    valuestack_drop(2);

    return ret;
}
