#include "symtab.h"

#include "types.h"
#include "valuestack.h"

#include <string.h>

typedef struct {
    StackIndex index;
} SymbolTable;

static SymbolTable symtab;

void symtab_init()
{
    symtab.index = PUSH(nil_make());
}

Pointer symtab_find(const char* s)
{
    // No allocations going on in here, so safe to use pointers.
    for (Pointer ptr = GET(symtab.index);
         ptr.type != Type_nil;
         ptr = pair_get(ptr, 1)) {

        Pointer sym = pair_get(ptr, 0);
        if (strcmp(s, symbol_get(sym)) == 0) {
            return sym;
        }
    }
    return nil_make();
}

void symtab_add(StackIndex symIndex)
{
    Pointer symbol = GET(symIndex);
    symbol.type = Type_symbol; // it starts as a string.

    SET(symIndex, symbol);

    SET(symtab.index, pair_make(symIndex, symtab.index));
}

Pointer symtab_insert(Pointer symbol)
{
    Pointer ret = symtab_find(symbol_get(symbol));
    if (ret.type == Type_nil) {
        symtab_add(PUSH(symbol));
        ret = POP();
    }
    return ret;
}
