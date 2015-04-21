#include "valuestack.h"

#include "debug.h"

#include <stdlib.h>
#include <stdio.h>

typedef struct {
    int         size;
    int         top;    // first empty slot
    Pointer*    slot;
} ValueStack;

static ValueStack vs;

void valuestack_init(int stackSize)
{
    vs.size = stackSize;
    vs.top  = 0;
    vs.slot = (Pointer*) (malloc(sizeof(Pointer) * stackSize));
}

StackIndex valuestack_push(Pointer ptr)
{
    if (vs.top >= vs.size) {
        valuestack_dump();
    }
    ASSERT(vs.top < vs.size, "Value stack overflow");

    int index = vs.top++;
    vs.slot[index] = ptr;
    return index;
}

StackIndex valuestack_reserve()
{
    return valuestack_push(nil_make());
}

Pointer valuestack_pop()
{
    ASSERT(vs.top >= 0, "Value stack underflow");

    int index = --vs.top;
    return vs.slot[index];
}

StackIndex valuestack_popTo(StackIndex index)
{
    // Can't pop upwards. New top must be <= vs.top.
    ASSERT((index >= 0) && (index <= vs.top),
        "New stack top %d must be in (0, %d)", index, vs.top);

    vs.top = index;
    return index;
}

StackIndex valuestack_drop(int howMany)
{
    ASSERT(howMany >= 0, "Can't drop %d values off the stack", howMany);
    ASSERT(howMany <= vs.top, "Stack has %d values, trying to drop %d\n",
        vs.top, howMany);

    vs.top -= howMany;
    return vs.top;
}

Pointer valuestack_get(StackIndex index)
{
    ASSERT((index >= 0) && (index < vs.top),
        "Reading outside of stack. Index %d not in (0, %d]", index, vs.top);
    return vs.slot[index];
}

Pointer valuestack_set(StackIndex index, Pointer ptr)
{
    // No reason to be writing off the top of the stack.
    ASSERT((index >= 0) && (index < vs.top),
        "Writing outside of stack. Index %d not in (0, %d]", index, vs.top);
    vs.slot[index] = ptr;
    return ptr;
}

void valuestack_swapHeaps()
{
    for (StackIndex index = 0; index < vs.top; index++) {
        vs.slot[index] = pointer_copy(vs.slot[index]);
    }
}

StackIndex valuestack_top()
{
    return vs.top;
}

void valuestack_dump()
{
    for (StackIndex index = 0; index < vs.top; index++) {
        printf("STACK[%d]:", index);
        print(vs.slot[index]);
    }
}
