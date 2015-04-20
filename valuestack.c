#include "valuestack.h"

#include "debug.h"

#include <stdlib.h>

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
    ASSERT(vs.top < vs.size, "Value stack overflow");

    int index = vs.top++;
    vs.slot[index] = ptr;
    return index;
}

StackIndex valuestack_popTo(StackIndex index)
{
    // Can't pop upwards. New top must be <= vs.top.
    ASSERT((index >= 0) && (index <= vs.top),
        "New stack top %d must be in (0, %d)", index, vs.top);

    vs.top = index;
    return index;
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
