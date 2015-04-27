#include "gc.h"

#include "allocator.h"
#include "valuestack.h"

#include <stdio.h>

// Use this to prevent re-entering the GC when DEBUG_GC_EVERY_ALLOC is active
int in_gc = 0;

void gc_run()
{
    if (in_gc) {
        return;
    }
    in_gc = 1;

    int before = allocator_bytesAvailable();

    allocator_swapHeaps();
    valuestack_swapHeaps();

    int after = allocator_bytesAvailable();
    fprintf(stderr, "Garbage collected: %d bytes freed.\n", after - before);
    fprintf(stderr, "Heap: %d used, %d free. Stack: %d used.\n",
        allocator_bytesUsed(), after, valuestack_top());

    in_gc = 0;
}
