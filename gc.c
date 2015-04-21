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
    fprintf(stderr, "Garbage collected %d -> %d: %d bytes freed\n", before, after, after - before);
}
