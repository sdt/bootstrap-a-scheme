#include "allocator.h"

#include <stdlib.h>

#define ALIGNMENT_BITS  2
#define ALIGNMENT       (1<<(ALIGNMENT_BITS))
#define ALIGNMENT_MASK  (ALIGNMENT-1)
#define ALIGN(size)     (((size)+ALIGNMENT-1)&~(ALIGNMENT_MASK))

struct _Allocator {
    byte* pool[2];
    int activePool;
    int poolSize;
    int bytesUsed;
};

Allocator* allocator_create(int poolSize)
{
    Allocator* allocator = (Allocator*) malloc(sizeof(Allocator) + 2*poolSize);

    allocator->pool[0] = (byte*)(allocator + 1);
    allocator->pool[1] = (byte*)(allocator->pool[0] + poolSize);
    allocator->activePool = 0;
    allocator->poolSize = poolSize;
    allocator->bytesUsed = 0;

    return allocator;
}

void allocator_delete(Allocator* allocator)
{
    free(allocator);
}

byte* allocator_alloc(Allocator* allocator, int size)
{
    size = ALIGN(size);
    int bytesAvailable = allocator->poolSize - allocator->bytesUsed;
    if (bytesAvailable < size) {
        return NULL;
    }

    byte* block = allocator->pool[allocator->activePool] + allocator->bytesUsed;
    allocator->bytesUsed += size;
    return block;
}

void allocator_swap(Allocator* allocator)
{
    allocator->activePool ^= 1;
    allocator->bytesUsed = 0;
}

unsigned allocator_getOffset(Allocator* allocator, byte* pointer)
{
    return (pointer - allocator->pool[0]) >> ALIGNMENT_BITS;
}

byte* allocator_getPointer(Allocator* allocator, unsigned offset)
{
    return allocator->pool[0] + (offset << ALIGNMENT_BITS);

}
