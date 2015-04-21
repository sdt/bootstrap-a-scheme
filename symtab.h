#ifndef INCLUDE_SYMTAB_H
#define INCLUDE_SYMTAB_H

#include "valuestack.h"

void symtab_init();
Pointer symtab_find(const char* s);
void symtab_add(StackIndex index);

#endif // INCLUDE_SYMTAB_H
