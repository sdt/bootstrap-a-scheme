#include "gc.h"

#include "allocator.h"
#include "debug.h"
#include "valuestack.h"

#include <stdio.h>

// Use this to prevent re-entering the GC when DEBUG_GC_EVERY_ALLOC is active
int in_gc = 0;

void gc_run_recursive()
{
    allocator_swapHeaps();
    valuestack_swapHeaps();
}

static Pointer moveObject(Pointer ptr)
{
    if (!type_isObject(ptr.type)) {
        return ptr;
    }

    Pointer ret = pointer_move(ptr);
    unsigned offset = ret.offset;

    while (!allocator_isOffsetAtEnd(offset)) {
        offset = value_fixup(offset);
    }

    return ret;
}

void gc_run_two_fingers()
{
    allocator_swapHeaps();

    int n = valuestack_top();
    for (int i = 0; i < n; i++) {
        Pointer ptr = valuestack_get(i);
        valuestack_set(i, moveObject(ptr));
    }
}


void gc_run()
{
    if (in_gc) {
        return;
    }
    in_gc = 1;

    int before = allocator_bytesAvailable();

    //gc_run_recursive();
    gc_run_two_fingers();

    int after = allocator_bytesAvailable();
    fprintf(stderr, "Garbage collected: %d bytes freed.\n", after - before);
    fprintf(stderr, "Heap: %d used, %d free. Stack: %d used.\n",
            allocator_bytesUsed(), after, valuestack_top());

    in_gc = 0;
}
