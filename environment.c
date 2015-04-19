#include "environment.h"
#include "exception.h"
#include "types.h"

// Environment is a list of (symbol => value) pairs
// (symbols, outer)

Pointer env_make(Pointer outer)
{
    return pair_make(nil_make(), outer);
}

void env_set(Pointer env, Pointer symbol, Pointer value)
{
    // For now, just push this onto the front. Don't worry if we've already
    // got this symbol.
    Pointer symbols = pair_get(env, 0);
    Pointer entry = pair_make(symbol, value);
    symbols = pair_make(entry, symbols);
    pair_set(env, 0, symbols);
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
    throw("\"%s\" not found", symbol_get(symbol));
    return nil_make();
}
