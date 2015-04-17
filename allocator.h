#ifndef INCLUDE_ALLOCATOR_H
#define INCLUDE_ALLOCATOR_H

#include "forward.h"

extern Allocator*   allocator_create(int poolSize);
extern void         allocator_delete(Allocator* allocator);
extern byte*        allocator_alloc(Allocator* allocator, int size);
extern void         allocator_swap(Allocator* allocator);
extern unsigned     allocator_getOffset(Allocator* allocator, byte* pointer);
extern byte*        allocator_getPointer(Allocator* allocator, unsigned offset);

#endif // INCLUDE_ALLOCATOR_H
