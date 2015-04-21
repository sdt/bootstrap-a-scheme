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
extern void         valuestack_dump();

// These probably should go elsewhere.
#define GET(stackIndex)                 valuestack_get(stackIndex)
#define SET(stackIndex, ptr)            valuestack_set(stackIndex, ptr)
#define PAIR_GET(stackIndex, which)     pair_get(GET(stackIndex), which)
#define PUSH(ptr)                       valuestack_push(ptr)
#define POP()                           valuestack_pop()
#define DROP(n)                         valuestack_drop(n)
#define NTH(stackIndex, n)              list_nth(GET(stackIndex), n)
#define RESERVE()                       valuestack_reserve()

#endif // INCLUDE_VALUESTACK_H
