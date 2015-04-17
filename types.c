#include "types.h"
#include "allocator.h"

#include <assert.h>
#include <stdlib.h>

#define ALLOC(allocator, type) ((type*)allocator_alloc(allocator, sizeof(type)))
#define VALUE(allocator, ptr, type) \
            (type*)allocator_getPointer(allocator, (ptr).offset);

typedef struct {
    byte* relocated;
} value_base;

typedef struct {
    value_base base;
    int value;
} value_integer;

typedef struct {
    value_base base;
    pointer car;
    pointer cdr;
} value_pair;

static pointer make_pointer(Allocator* allocator, type type, byte* raw)
{
    pointer ptr = { type, allocator_getOffset(allocator, raw) };
    return ptr;
}

pointer make_integer(Allocator* allocator, int value)
{
    value_integer* raw = ALLOC(allocator, value_integer);

    raw->base.relocated = NULL;
    raw->value = value;

    return make_pointer(allocator, type_integer, (byte*) raw);
}

int get_integer(Allocator* allocator, pointer ptr)
{
    assert(ptr.type == type_integer);
    value_integer* raw = VALUE(allocator, ptr, value_integer);
    return raw->value;
}

pointer make_nil()
{
    pointer ptr = { type_nil, 0 };
    return ptr;
}

pointer make_pair(Allocator* allocator, pointer car, pointer cdr)
{
    value_pair* raw = ALLOC(allocator, value_pair);

    raw->base.relocated = NULL;
    raw->car = car;
    raw->cdr = cdr;

    return make_pointer(allocator, type_pair, (byte*) raw);
}

pointer get_car(Allocator* allocator, pointer ptr)
{
    assert(ptr.type == type_pair);
    value_pair* raw = VALUE(allocator, ptr, value_pair);
    return raw->car;
}

pointer get_cdr(Allocator* allocator, pointer ptr)
{
    assert(ptr.type == type_pair);
    value_pair* raw = VALUE(allocator, ptr, value_pair);
    return raw->cdr;
}

