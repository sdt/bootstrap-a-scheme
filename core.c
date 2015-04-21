#include "core.h"
#include "environment.h"
#include "exception.h"
#include "gc.h"
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
    StackIndex symIndex = PUSH(nil_make());
    StackIndex valIndex = RESERVE();
    StackIndex envIndex = PUSH(env_root());

    for (int i = 0; i < Core_COUNT; i++) {
        Handler* h = &handlerTable[i];
        SET(symIndex, symbol_make(h->symbol));
        SET(valIndex, builtin_make(i));

        env_set(envIndex, symIndex, valIndex);
    }

    DROP(2);
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
    int argsCount = countArgs(GET(argsIndex)); \
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
    for (Pointer args = GET(argsIndex);
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

    StackIndex carIndex = PUSH(NTH(argsIndex, 0));
    StackIndex cdrIndex = PUSH(NTH(argsIndex, 1));

    Pointer ret = pair_make(carIndex, cdrIndex);

    DROP(2);

    return ret;
}

HANDLER(car)
{
    // There's no allocations going on in here, so it's safe to walk the
    // raw pointers.
    CHECK_ARGS_COUNT(1);
    Pointer args = GET(argsIndex);

    Pointer pair = ARG(args, pair);

    return pair_get(pair, 0);
}

HANDLER(cdr)
{
    // There's no allocations going on in here, so it's safe to walk the
    // raw pointers.
    CHECK_ARGS_COUNT(1);
    Pointer args = GET(argsIndex);

    Pointer pair = ARG(args, pair);

    return pair_get(pair, 1);
}

HANDLER(dump)
{
    valuestack_dump();
    return nil_make();
}

HANDLER(equals)
{
    CHECK_ARGS_COUNT(2);

    return boolean_make(integer_get(NTH(argsIndex, 0))
                     == integer_get(NTH(argsIndex, 1)));
}

HANDLER(gc)
{
    gc_run();
    return nil_make();
}

HANDLER(isEmpty)
{
    // There's no allocations going on in here, so it's safe to walk the
    // raw pointers.
    CHECK_ARGS_COUNT(1);
    Pointer args = GET(argsIndex);

    Pointer a = ARGPTR(args);
    return boolean_make(a.type == Type_nil);
}

HANDLER(lt)
{
    CHECK_ARGS_COUNT(2);

    return boolean_make(integer_get(NTH(argsIndex, 0))
                     <  integer_get(NTH(argsIndex, 1)));
}

HANDLER(mul)
{
    // There's no allocations going on in here, so it's safe to walk the
    // raw pointers.
    int product = 1;
    for (Pointer args = GET(argsIndex);
         args.type != Type_nil; args = pair_get(args, 1)) {

        Pointer intPtr = pair_get(args, 0);
        CHECK_TYPE(intPtr, integer);

        product *= integer_get(intPtr);
    }

    return integer_make(product);
}

