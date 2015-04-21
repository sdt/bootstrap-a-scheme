#ifndef INCLUDE_ALLOCATOR_H
#define INCLUDE_ALLOCATOR_H

typedef unsigned char byte;

extern void         allocator_init(int heapSize);
extern void         allocator_deinit();
extern byte*        allocator_alloc(int size);
extern void         allocator_swapHeaps();
extern int          allocator_bytesAvailable();
extern int          allocator_bytesUsed();
extern unsigned     allocator_getOffset(byte* pointer);
extern byte*        allocator_getPointer(unsigned offset);

#endif // INCLUDE_ALLOCATOR_H
