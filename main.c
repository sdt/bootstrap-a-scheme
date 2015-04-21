#include "allocator.h"
#include "debug.h"
#include "core.h"
#include "environment.h"
#include "exception.h"
#include "input.h"
#include "reader.h"
#include "symtab.h"
#include "types.h"
#include "valuestack.h"

#include <stdio.h>
#include <string.h>

static Pointer eval_list(Pointer list, Pointer env);
Pointer eval(Pointer ast, Pointer env);

int main(int argc, char* argv[])
{
    int heapSize = 1 * 1024;
    allocator_init(heapSize);
    valuestack_init(heapSize / 16);
    env_init();
    symtab_init();
    types_init();
    core_init();

    StackIndex prevTop = valuestack_top();

    char* input;
    while ((input = getInput("bas> ")) != NULL) {
        StackIndex top = valuestack_top();
        ASSERT(top == prevTop, "Value stack out of whack! Was %d, is %d",
            prevTop, top);
        prevTop = top;

        if (setjmp(*exception_buf()) != 0) {
            fprintf(stderr, "%s\n", exception_msg());
            valuestack_popTo(top);
            continue;
        }

        Pointer list = readLine(input);
        print(eval(list, env_root()));
    }

    return 0;
}

Pointer eval(Pointer ast, Pointer env)
{
    Pointer op, args;
    const char* sym;

    ast = pointer_follow(ast);
    env = pointer_follow(env);

    switch (ast.type) {
    default:
        return ast;

    case Type_symbol:
        return env_get(env, ast);

    case Type_pair:
        op = pair_get(ast, 0);

        if (op.type == Type_symbol) {
            args = pair_get(ast, 1);
            sym = symbol_get(op);

            if (strcmp(sym, "define") == 0) {
                Pointer symbol = pair_get(args, 0); args = pair_get(args, 1);
                Pointer value  = pair_get(args, 0); args = pair_get(args, 1);
                value = eval(value, env);
                env_set(pointer_follow(env), symbol, eval(value, env));
                return pointer_follow(value);
            }

            if (strcmp(sym, "if") == 0) {
                Pointer cond = pair_get(args, 0); args = pair_get(args, 1);
                Pointer thenRet = pair_get(args, 0); args = pair_get(args, 1);
                Pointer elseRet = (args.type == Type_pair) ? pair_get(args, 0)
                                                        : args;
                if (!pointer_isFalse(eval(cond, env))) {
                    return eval(thenRet, env);
                }
                else {
                    return eval(elseRet, env);
                }
            }

            if (strcmp(sym, "lambda") == 0) {
                Pointer params = pair_get(args, 0); args = pair_get(args, 1);
                Pointer body   = pair_get(args, 0); args = pair_get(args, 1);
                return lambda_make(params, body, env);
            }
        }

        // If we get here, no special forms applied.
        ast  = eval_list(ast, env);
        op   = pair_get(ast, 0);
        args = pair_get(ast, 1);

        if (op.type == Type_builtin) {
            return builtin_apply(op, args, env);
        }
        if (op.type == Type_lambda) {
            return lambda_apply(op, args, env);
        }
        THROW("%s is not applicable", type_name(op.type));

        return ast;
    }
}

static Pointer eval_list(Pointer list, Pointer env)
{
    if (list.type == Type_nil) {
        return list;
    }

    list = pointer_follow(list);
    env  = pointer_follow(env);

    Pointer car = eval(pair_get(list, 0), env = pointer_follow(env));
    list = pointer_follow(list);
    env  = pointer_follow(env);

    return pair_make(pointer_follow(car),
                     eval_list(pair_get(list, 1), pointer_follow(env)));
}
