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
extern unsigned     allocator_advanceOffset(unsigned offset, unsigned size);
extern byte*        allocator_getPointer(unsigned offset);
extern int          allocator_isOffsetActive(unsigned offset);
extern int          allocator_isOffsetAtEnd(unsigned offset);
extern int          allocator_roundUp(int size);

#endif // INCLUDE_ALLOCATOR_H
