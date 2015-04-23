#include "core.h"

#include "args.h"
#include "environment.h"
#include "exception.h"
#include "gc.h"
#include "types.h"
#include "util.h"
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

    DROP(3);
}

Pointer core_apply(CoreHandlerId id, StackIndex argsIndex, StackIndex envIndex)
{
    Handler* h = &handlerTable[id];
    return h->f(h->symbol, argsIndex, envIndex);
}

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
    GET_ARGS_EXACTLY(2);

    Pointer ret = pair_make(ARG_INDEX(0), ARG_INDEX(1));

    DROP_ARGS();
    return ret;
}

HANDLER(car)
{
    GET_ARGS_EXACTLY(1);
    ARG_CHECKTYPE(0, pair, "arg");

    Pointer ret = pair_get(GET(ARG_INDEX(0)), 0);

    DROP_ARGS();
    return ret;
}

HANDLER(cdr)
{
    GET_ARGS_EXACTLY(1);
    ARG_CHECKTYPE(0, pair, "arg");

    Pointer ret = pair_get(GET(ARG_INDEX(0)), 1);

    DROP_ARGS();
    return ret;
}

HANDLER(dump)
{
    valuestack_dump();
    return nil_make();
}

HANDLER(equals)
{
    GET_ARGS_EXACTLY(2);
    ARG_CHECKTYPE(0, integer, "arg 1");
    ARG_CHECKTYPE(1, integer, "arg 2");

    Pointer ret =  boolean_make(integer_get(GET(ARG_INDEX(0)))
                             == integer_get(GET(ARG_INDEX(1))));

    DROP_ARGS();
    return ret;
}

HANDLER(gc)
{
    gc_run();
    return nil_make();
}

HANDLER(isEmpty)
{
    GET_ARGS_EXACTLY(1);

    Pointer ret = boolean_make(GET(ARG_INDEX(0)).type == Type_nil);

    DROP_ARGS();
    return ret;
}

HANDLER(lt)
{
    GET_ARGS_EXACTLY(2);
    ARG_CHECKTYPE(0, integer, "arg 1");
    ARG_CHECKTYPE(1, integer, "arg 2");

    Pointer ret =  boolean_make(integer_get(GET(ARG_INDEX(0)))
                             <  integer_get(GET(ARG_INDEX(1))));

    DROP_ARGS();
    return ret;
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
