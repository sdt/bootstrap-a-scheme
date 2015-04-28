#include "allocator.h"

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

#define ALIGNMENT_BITS  2
#define ALIGNMENT       (1<<(ALIGNMENT_BITS))
#define ALIGNMENT_MASK  (ALIGNMENT-1)
#define ALIGN(size)     (((size)+ALIGNMENT-1)&~(ALIGNMENT_MASK))
#define ALIGN_PTR(ptr)  ((void*)ALIGN((long)(ptr)))

typedef struct {
    byte* heap[2];
    int activeHeap;
    int heapSize;
    int bytesUsed;
    byte* data;
} Allocator;

static Allocator allocator;

void allocator_init(int heapSize)
{
    allocator.data = (byte*) malloc(heapSize * 2 + ALIGNMENT - 1);
    allocator.heap[0] = ALIGN_PTR(allocator.data);
    allocator.heap[1] = allocator.heap[0] + heapSize;
    allocator.activeHeap = 0;
    allocator.heapSize = heapSize;
    allocator.bytesUsed = 0;
}

void allocator_deinit()
{
    free(allocator.data);
}

byte* allocator_alloc(int size)
{
    size = ALIGN(size);
    int bytesAvailable = allocator.heapSize - allocator.bytesUsed;
    if (bytesAvailable < size) {
        return NULL;
    }

    byte* block = allocator.heap[allocator.activeHeap] + allocator.bytesUsed;
    allocator.bytesUsed += size;
    return block;
}

void allocator_swapHeaps()
{
    allocator.activeHeap ^= 1;
    allocator.bytesUsed = 0;
#if DEBUG_WIPE_HEAP
    memset(allocator.heap[allocator.activeHeap], 'A', allocator.heapSize);
#endif
}

unsigned allocator_getOffset(byte* pointer)
{
    return (pointer - allocator.heap[0]) >> ALIGNMENT_BITS;
}

byte* allocator_getPointer(unsigned offset)
{
    return allocator.heap[0] + (offset << ALIGNMENT_BITS);
}

int allocator_isOffsetActive(unsigned offset)
{
    offset <<= ALIGNMENT_BITS;
    offset -= allocator.heapSize * allocator.activeHeap;
    return offset < allocator.heapSize;
}

int allocator_bytesAvailable()
{
    return allocator.heapSize - allocator.bytesUsed;
}

int allocator_bytesUsed()
{
    return allocator.bytesUsed;
}

void allocator_dumpInfo(FILE* fh)
{
    fprintf(fh, "%d-byte heaps @ (0x%p/0x%p) %d active, %d used\n",
        allocator.heapSize,
        allocator.heap[0],
        allocator.heap[1],
        allocator.activeHeap,
        allocator.bytesUsed);
}

