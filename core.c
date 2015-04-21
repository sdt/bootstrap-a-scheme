#include "core.h"
#include "environment.h"
#include "exception.h"
#include "types.h"
#include "valuestack.h"

#define HANDLER_ARGS const char* symbol, \
                     StackIndex argsIndex, \
                     StackIndex envIndex

typedef Pointer (HandlerFunc)(HANDLER_ARGS);
typedef struct {
    const char*  symbol;
    HandlerFunc* f;
} Handler;

#define HANDLER(ident) static Pointer Handler_##ident(HANDLER_ARGS)

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
    StackIndex symIndex = valuestack_push(nil_make());

    for (int i = 0; i < Core_COUNT; i++) {
        Handler* h = &handlerTable[i];
        valuestack_set(symIndex, symbol_make(h->symbol));
        Pointer value = builtin_make(i);
        env_set(valuestack_get(env_root()), valuestack_get(symIndex), value);
    }

    valuestack_drop(1);
}

Pointer core_apply(CoreHandlerId id, StackIndex argsIndex, StackIndex envIndex)
{
    Handler* h = &handlerTable[id];
    return h->f(h->symbol, argsIndex, envIndex);
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
    int argsCount = countArgs(valuestack_get(argsIndex)); \
    if (argsCount != (expected)) { \
        THROW("%s: %d arg%s expected, %d provided", \
            symbol, expected, PLURAL(expected), argsCount); \
    }

#define ARGPTR(args)    pair_get(args, 0); args = pair_get(args, 1);
#define ARG(args, type)     type_check(pair_get(args, 0), Type_##type); \
                            args = pair_get(args, 1);

#define CHECK_TYPE(ptr, expected) \
    if (ptr.type != Type_##expected) { \
        THROW("%s: expected %s, got %s", \
            symbol, #expected, type_name(ptr.type)); }


HANDLER(add)
{
    // There's no allocations going on in here, so it's safe to walk the
    // raw pointers.
    int sum = 0;
    for (Pointer args = valuestack_get(argsIndex);
         args.type != Type_nil; args = pair_get(args, 1)) {

        Pointer intPtr = pair_get(args, 0);
        CHECK_TYPE(intPtr, integer);

        sum += integer_get(intPtr);
    }

    return integer_make(sum);
}

HANDLER(cons)
{
    // There's no allocations going on in here, so it's safe to walk the
    // raw pointers.
    CHECK_ARGS_COUNT(2);
    Pointer args = valuestack_get(argsIndex);

    Pointer a = ARGPTR(args);
    Pointer b = ARGPTR(args);

    return pair_make(a, b);
}

HANDLER(car)
{
    // There's no allocations going on in here, so it's safe to walk the
    // raw pointers.
    CHECK_ARGS_COUNT(1);
    Pointer args = valuestack_get(argsIndex);

    Pointer pair = ARG(args, pair);

    return pair_get(pair, 0);
}

HANDLER(cdr)
{
    // There's no allocations going on in here, so it's safe to walk the
    // raw pointers.
    CHECK_ARGS_COUNT(1);
    Pointer args = valuestack_get(argsIndex);

    Pointer pair = ARG(args, pair);

    return pair_get(pair, 1);
}

HANDLER(isEmpty)
{
    // There's no allocations going on in here, so it's safe to walk the
    // raw pointers.
    CHECK_ARGS_COUNT(1);
    Pointer args = valuestack_get(argsIndex);

    Pointer a = ARGPTR(args);
    return boolean_make(a.type == Type_nil);
}
