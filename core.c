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
        THROW("%s: arg is %s, expected %s", \
            symbol, type_name(ptr.type), #expected); }


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
    DROP_RET(pair_make(ARG_INDEX(0), ARG_INDEX(1)));
}

HANDLER(car)
{
    GET_ARGS_EXACTLY(1);
    ARG_CHECKTYPE(0, pair, "arg");
    DROP_RET(pair_get(GET(ARG_INDEX(0)), 0));
}

HANDLER(cdr)
{
    GET_ARGS_EXACTLY(1);
    ARG_CHECKTYPE(0, pair, "arg");
    DROP_RET(pair_get(GET(ARG_INDEX(0)), 1));
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

    DROP_RET(boolean_make(integer_get(GET(ARG_INDEX(0)))
                       == integer_get(GET(ARG_INDEX(1)))));
}

HANDLER(gc)
{
    gc_run();
    return nil_make();
}

HANDLER(isEmpty)
{
    GET_ARGS_EXACTLY(1);
    DROP_RET(boolean_make(GET(ARG_INDEX(0)).type == Type_nil));
}

HANDLER(lt)
{
    GET_ARGS_EXACTLY(2);
    ARG_CHECKTYPE(0, integer, "arg 1");
    ARG_CHECKTYPE(1, integer, "arg 2");
    DROP_RET(boolean_make(integer_get(GET(ARG_INDEX(0)))
                       <  integer_get(GET(ARG_INDEX(1)))));
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

HANDLER(sub)
{
    Pointer args = GET(argsIndex);
    if (args.type == Type_nil) {
        THROW("%s: at least 1 arg expected, 0 provided", symbol);
    }

    Pointer intPtr = pair_get(args, 0);
    CHECK_TYPE(intPtr, integer);
    int result = integer_get(intPtr);

    args = pair_get(args, 1);
    if (args.type == Type_nil) {
        // Only one argument, negate it.
        return integer_make(- result);
    }

    for ( ; args.type != Type_nil; args = pair_get(args, 1)) {

        Pointer intPtr = pair_get(args, 0);
        CHECK_TYPE(intPtr, integer);

        result -= integer_get(intPtr);
    }

    return integer_make(result);
}

HANDLER(vector)
{
    GET_ARGS_ALL();

    StackIndex vecIndex = PUSH(vector_make(ARGS_COUNT()));
    for (int i = 0; i < ARGS_COUNT(); i++) {
        vector_set(vecIndex, i, GET(ARG_INDEX(i)));
    }

    Pointer vec = POP();
    DROP_ARGS();
    return vec;
}

HANDLER(vectorGet)
{
    GET_ARGS_EXACTLY(2);
    ARG_CHECKTYPE(0, vector,  "arg 1");
    ARG_CHECKTYPE(1, integer, "arg 2");

    DROP_RET(vector_get(GET(ARG_INDEX(0)), integer_get(GET(ARG_INDEX(1)))));
}

HANDLER(vectorSet)
{
    GET_ARGS_EXACTLY(3);
    ARG_CHECKTYPE(0, vector,  "arg 1");
    ARG_CHECKTYPE(1, integer, "arg 2");

    DROP_RET(vector_set(ARG_INDEX(0),
                        integer_get(GET(ARG_INDEX(1))),
                        GET(ARG_INDEX(2))));
}
