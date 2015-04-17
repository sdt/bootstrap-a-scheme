#include "types.h"
#include "allocator.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#define ALLOC(type)      ((type*)allocate_value(sizeof(type)))
#define VALUE(ptr, type) (type*)allocator_getPointer((ptr).offset);

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

static pointer make_pointer(type type, byte* raw)
{
    pointer ptr = { type, allocator_getOffset(raw) };
    return ptr;
}

static value_base* allocate_value(int size)
{
    value_base* base = (value_base*) allocator_alloc(size);
    base->relocated = nil;
    return base;
}

pointer make_integer(int value)
{
    value_integer* raw = ALLOC(value_integer);
    raw->value = value;

    return make_pointer(type_integer, (byte*) raw);
}

int get_integer(pointer ptr)
{
    assert(ptr.type == type_integer);
    value_integer* raw = VALUE(ptr, value_integer);
    return raw->value;
}

pointer make_nil()
{
    return nil;
}

pointer make_pair(pointer car, pointer cdr)
{
    value_pair* raw = ALLOC(value_pair);
    raw->car = car;
    raw->cdr = cdr;

    return make_pointer(type_pair, (byte*) raw);
}

pointer get_car(pointer ptr)
{
    assert(ptr.type == type_pair);
    value_pair* raw = VALUE(ptr, value_pair);
    return raw->car;
}

pointer get_cdr(pointer ptr)
{
    assert(ptr.type == type_pair);
    value_pair* raw = VALUE(ptr, value_pair);
    return raw->cdr;
}

void set_car(pointer ptr, pointer car)
{
    assert(ptr.type == type_pair);
    value_pair* raw = VALUE(ptr, value_pair);
    raw->car = car;
}

void set_cdr(pointer ptr, pointer cdr)
{
    assert(ptr.type == type_pair);
    value_pair* raw = VALUE(ptr, value_pair);
    raw->cdr = cdr;
}

pointer copy(pointer ptr)
{
    if (ptr.type == type_nil) {
        return ptr;
    }

    value_base* base = VALUE(ptr, value_base);
    if (base->relocated.type == type_nil) {
        switch (ptr.type) {
            case type_integer: {
                base->relocated =
                    make_integer(get_integer(ptr));
                break;
            }
            case type_pair: {
                base->relocated = make_pair(nil, nil);
                set_cdr(base->relocated,
                    copy(get_cdr(ptr)));
                set_car(base->relocated,
                    copy(get_car(ptr)));
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

pointer stop_and_copy(pointer ptr)
{
    int before = allocator_bytesAvailable();

    allocator_swapHeaps();
    ptr = copy(ptr);

    int after = allocator_bytesAvailable();
    fprintf(stderr, "Garbage collected %d -> %d: %d bytes freed\n", before, after, after - before);

    return ptr;
}

void print_value(pointer ptr)
{
    switch (ptr.type) {
        case type_nil: {
            printf("nil");
            break;
        }

        case type_integer: {
            printf("%d", get_integer(ptr));
            break;
        }

        case type_pair: {
            printf("[");
            print_value(get_car(ptr));
            printf(", ");
            print_value(get_cdr(ptr));
            printf("]");
            break;
        }
    }
}

void print(pointer ptr)
{
    print_value(ptr);
    printf("\n");
}
