#include "gc.h"

#include "allocator.h"
#include "valuestack.h"

#include <stdio.h>

void gc_run()
{
    int before = allocator_bytesAvailable();

    allocator_swapHeaps();
    valuestack_swapHeaps();

    int after = allocator_bytesAvailable();
    fprintf(stderr, "Garbage collected: %d bytes freed.\n", after - before);
    fprintf(stderr, "Heap: %d used, %d free.\n", allocator_bytesUsed(), after);
}
