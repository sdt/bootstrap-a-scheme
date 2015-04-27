#ifndef INCLUDE_SYMTAB_H
#define INCLUDE_SYMTAB_H

#include "valuestack.h"

extern void     symtab_init();
extern void     symtab_add(StackIndex index);
extern Pointer  symtab_find(const char* s);
extern Pointer  symtab_insert(Pointer symbol);

#endif // INCLUDE_SYMTAB_H
