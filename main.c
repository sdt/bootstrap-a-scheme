#include "allocator.h"
#include "core.h"
#include "environment.h"
#include "exception.h"
#include "input.h"
#include "reader.h"
#include "types.h"

#include <stdio.h>
#include <string.h>

static Pointer eval_list(Pointer list, Pointer env);
Pointer eval(Pointer ast, Pointer env);

int main(int argc, char* argv[])
{
    allocator_init(1 * 1024);
    types_init();
    core_init();

    char* input;
    while ((input = getInput("bas> ")) != NULL) {
        EXCEPTION_SCOPE;

        Pointer list = readLine(input);
        print(eval(list, getRootEnv()));
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

            // special forms
        }

        // If we get here, no special forms applied.
        ast  = eval_list(ast, env);
        op   = pair_get(ast, 0);
        args = pair_get(ast, 1);

        if (op.type == Type_builtin) {
            return builtin_apply(op, args, env);
        }
        throw("%s is not applicable", type_name(op.type));

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
