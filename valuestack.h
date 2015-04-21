#ifndef INCLUDE_VALUESTACK_H
#define INCLUDE_VALUESTACK_H

#include "basetypes.h"
#include "types.h"

extern void         valuestack_init(int stackSize);
extern StackIndex   valuestack_push(Pointer ptr);
extern StackIndex   valuestack_reserve();
extern Pointer      valuestack_pop();
extern StackIndex   valuestack_drop(int howMany);
extern StackIndex   valuestack_popTo(StackIndex index);
extern Pointer      valuestack_get(StackIndex index);
extern Pointer      valuestack_set(StackIndex index, Pointer ptr);
extern void         valuestack_swapHeaps();
extern StackIndex   valuestack_top();

#endif // INCLUDE_VALUESTACK_H
