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
    symtab.index = valuestack_push(nil_make());
}

Pointer symtab_find(const char* s)
{
    // No allocations going on in here, so safe to use pointers.
    for (Pointer ptr = valuestack_get(symtab.index);
         ptr.type != Type_nil;
         ptr = pair_get(ptr, 1)) {

        Pointer sym = pair_get(ptr, 0);
        if (strcmp(s, symbol_get(sym)) == 0) {
            return sym;
        }
    }
    return nil_make();
}

void symtab_add(StackIndex index)
{
    Pointer symbol = valuestack_get(index);
    symbol.type = Type_symbol; // it starts as a string.
    valuestack_set(index, symbol);

    Pointer pair = pair_make(symbol, valuestack_get(symtab.index));
    valuestack_set(symtab.index, pair);
}
