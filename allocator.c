#include <stdlib.h>

typedef unsigned char byte;

typedef struct {
    byte* pool[2];
    int activePool;
    int poolSize;
    int bytesUsed;
} Allocator;

Allocator* allocator_init(int poolSize)
{
    Allocator* allocator = (Allocator*) malloc(sizeof(Allocator) + 2*poolSize);

    allocator->pool[0] = (byte*)(allocator + 1);
    allocator->pool[1] = (byte*)(allocator->pool[0] + poolSize);
    allocator->activePool = 0;
    allocator->poolSize = poolSize;
    allocator->bytesUsed = 0;

    return allocator;
}

void allocator_free(Allocator* allocator)
{
    free(allocator);
}

byte* allocator_alloc(Allocator* allocator, int size)
{
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
