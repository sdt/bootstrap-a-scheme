#include "core.h"
#include "environment.h"
#include "exception.h"
#include "types.h"

typedef Pointer (HandlerFunc)(const char* symbol, Pointer args, Pointer env);
typedef struct {
    const char*  symbol;
    HandlerFunc* f;
} Handler;

#define HANDLER(ident) static Pointer \
    Handler_##ident(const char* symbol, Pointer args, Pointer env)

#define X(sym, ident) HANDLER(ident);
    CORE_HANDLERS_XLIST
#undef X

static Handler handlerTable[] = {
    #define X(sym, ident) { sym, Handler_##ident },
        CORE_HANDLERS_XLIST
    #undef X
};

void core_init()
{
    for (int i = 0; i < Core_COUNT; i++) {
        Handler* h = &handlerTable[i];
        Pointer symbol = symbol_make(h->symbol);
        Pointer value = builtin_make(i);
        env_set(env_root(), symbol, value);
    }
}

Pointer core_apply(CoreHandlerId id, Pointer args, Pointer env)
{
    Handler* h = &handlerTable[id];
    return h->f(h->symbol, args, env);
}

static int countArgs(Pointer args)
{
    int len = 0;
    for ( ; args.type == Type_pair; args = pair_get(args, 1)) {
        len++;
    }
    return len;
}

#define PLURAL(n)   (&"s"[(n)==1])

#define CHECK_ARGS_COUNT(expected) \
    int argsCount = countArgs(args); \
    if (argsCount != (expected)) { \
        THROW("\"%s\" expects %d arg%s, %d provided", \
            symbol, expected, PLURAL(expected), argsCount); \
    }

#define ARGPTR()    pair_get(args, 0); args = pair_get(args, 1);
#define ARG(type)   type_check(pair_get(args, 0), Type_##type); \
                    args = pair_get(args, 1);

HANDLER(add)
{
    CHECK_ARGS_COUNT(2);
    Pointer a = ARG(integer);
    Pointer b = ARG(integer);
    return integer_make(integer_get(a) + integer_get(b));
}

HANDLER(cons)
{
    CHECK_ARGS_COUNT(2);
    Pointer a = ARGPTR();
    Pointer b = ARGPTR();

    return pair_make(a, b);
}

HANDLER(car)
{
    CHECK_ARGS_COUNT(1);
    Pointer pair = ARG(pair);
    return pair_get(pair, 0);
}

HANDLER(cdr)
{
    CHECK_ARGS_COUNT(1);
    Pointer pair = ARG(pair);
    return pair_get(pair, 1);
}

HANDLER(isEmpty)
{
    CHECK_ARGS_COUNT(1);
    Pointer a = ARGPTR();
    return boolean_make(a.type == Type_nil);
}
