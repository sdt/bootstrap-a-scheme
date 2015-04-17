#include "types.h"
#include "allocator.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#define ALLOC(allocator, type) ((type*)allocate_value(allocator, sizeof(type)))
#define VALUE(allocator, ptr, type) \
            (type*)allocator_getPointer(allocator, (ptr).offset);

typedef struct {
    pointer relocated;
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

static pointer nil = { type_nil, 0 };

static pointer make_pointer(Allocator* allocator, type type, byte* raw)
{
    pointer ptr = { type, allocator_getOffset(allocator, raw) };
    return ptr;
}

static value_base* allocate_value(Allocator* allocator, int size)
{
    value_base* base = (value_base*) allocator_alloc(allocator, size);
    base->relocated = nil;
    return base;
}

pointer make_integer(Allocator* allocator, int value)
{
    value_integer* raw = ALLOC(allocator, value_integer);
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
    return nil;
}

pointer make_pair(Allocator* allocator, pointer car, pointer cdr)
{
    value_pair* raw = ALLOC(allocator, value_pair);
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

void set_car(Allocator* allocator, pointer ptr, pointer car)
{
    assert(ptr.type == type_pair);
    value_pair* raw = VALUE(allocator, ptr, value_pair);
    raw->car = car;
}

void set_cdr(Allocator* allocator, pointer ptr, pointer cdr)
{
    assert(ptr.type == type_pair);
    value_pair* raw = VALUE(allocator, ptr, value_pair);
    raw->cdr = cdr;
}

pointer copy(Allocator* allocator, pointer ptr)
{
    if (ptr.type == type_nil) {
        return ptr;
    }

    value_base* base = VALUE(allocator, ptr, value_base);
    if (base->relocated.type == type_nil) {
        switch (ptr.type) {
            case type_integer: {
                base->relocated =
                    make_integer(allocator, get_integer(allocator, ptr));
                break;
            }
            case type_pair: {
                base->relocated = make_pair(allocator, nil, nil);
                set_cdr(allocator, base->relocated,
                    copy(allocator, get_cdr(allocator, ptr)));
                set_car(allocator, base->relocated,
                    copy(allocator, get_car(allocator, ptr)));
                break;
            }
            default: {
                assert(0);
                break;
            }
        }
    }
    return base->relocated;
}

pointer stop_and_copy(Allocator* allocator, pointer ptr)
{
    int before = allocator_bytesAvailable(allocator);

    allocator_swap(allocator);
    ptr = copy(allocator, ptr);

    int after = allocator_bytesAvailable(allocator);
    fprintf(stderr, "Garbage collected %d -> %d: %d bytes freed\n", before, after, after - before);

    return ptr;
}

void print_value(Allocator* allocator, pointer ptr)
{
    switch (ptr.type) {
        case type_nil: {
            printf("nil");
            break;
        }

        case type_integer: {
            printf("%d", get_integer(allocator, ptr));
            break;
        }

        case type_pair: {
            printf("[");
            print_value(allocator, get_car(allocator, ptr));
            printf(", ");
            print_value(allocator, get_cdr(allocator, ptr));
            printf("]");
            break;
        }
    }
}

void print(Allocator* allocator, pointer ptr)
{
    print_value(allocator, ptr);
    printf("\n");
}

