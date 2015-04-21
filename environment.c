#include "environment.h"

#include "basetypes.h"
#include "exception.h"
#include "types.h"
#include "valuestack.h"

// Environment is a pair of (Map, outerEnv)
// Map is a list of (symbol, value) pairs

typedef struct {
    StackIndex rootIndex;
} EnvironmentData;

static EnvironmentData data;

void env_init()
{
    data.rootIndex = RESERVE();

    StackIndex mapIndex = PUSH(nil_make()); // TODO: fixed slot for nil
    SET(data.rootIndex, env_make(mapIndex));
    DROP(1);
}

StackIndex env_root()
{
    return data.rootIndex;
}

Pointer env_make(StackIndex outerIndex)
{
    StackIndex mapIndex = PUSH(nil_make()); // TODO: fixed slot for nil
    Pointer ret = pair_make(mapIndex, outerIndex);

    DROP(1);
    return ret;
}

void env_set(StackIndex envIndex, StackIndex symIndex, StackIndex valIndex)
{
    // For now, just push this onto the front. Don't worry if we've already
    // got this symbol.
    StackIndex newEntryIndex = PUSH(pair_make(symIndex, valIndex));
    StackIndex oldMapIndex   = PUSH(PAIR_GET(envIndex, 0));
    StackIndex newMapIndex   = PUSH(pair_make(newEntryIndex, oldMapIndex));

    pair_set(GET(envIndex), 0, GET(newMapIndex));

    DROP(3);
}

Pointer env_get(Pointer env, Pointer symbol)
{
    for ( ; env.type != Type_nil; env = pair_get(env, 1)) {
        for (Pointer symbols = pair_get(env, 0);
             symbols.type != Type_nil;
             symbols = pair_get(symbols, 1)) {

            Pointer entry = pair_get(symbols, 0);
            if (symbol.offset == pair_get(entry, 0).offset) {
                return pair_get(entry, 1);
            }
        }
    }
    THROW("\"%s\" not found", symbol_get(symbol));
    return nil_make();
}
