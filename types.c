#include "types.h"
#include "allocator.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#define ALLOC(type)      ((type*)allocate_value(sizeof(type)))
#define VALUE(ptr, type) (type*)allocator_getPointer((ptr).offset);

typedef struct {
    Pointer relocated;
} Value_base;

typedef struct {
    Value_base base;
    int value;
} Value_integer;

typedef struct {
    Value_base base;
    Pointer car;
    Pointer cdr;
} Value_pair;

static Pointer nil = { Type_nil, 0 };

static Pointer make_pointer(Type type, byte* raw)
{
    Pointer ptr = { type, allocator_getOffset(raw) };
    return ptr;
}

static Value_base* allocate_value(int size)
{
    Value_base* base = (Value_base*) allocator_alloc(size);
    base->relocated = nil;
    return base;
}

Pointer make_integer(int value)
{
    Value_integer* raw = ALLOC(Value_integer);
    raw->value = value;

    return make_pointer(Type_integer, (byte*) raw);
}

int get_integer(Pointer ptr)
{
    assert(ptr.type == Type_integer);
    Value_integer* raw = VALUE(ptr, Value_integer);
    return raw->value;
}

Pointer make_nil()
{
    return nil;
}

Pointer make_pair(Pointer car, Pointer cdr)
{
    Value_pair* raw = ALLOC(Value_pair);
    raw->car = car;
    raw->cdr = cdr;

    return make_pointer(Type_pair, (byte*) raw);
}

Pointer get_car(Pointer ptr)
{
    assert(ptr.type == Type_pair);
    Value_pair* raw = VALUE(ptr, Value_pair);
    return raw->car;
}

Pointer get_cdr(Pointer ptr)
{
    assert(ptr.type == Type_pair);
    Value_pair* raw = VALUE(ptr, Value_pair);
    return raw->cdr;
}

void set_car(Pointer ptr, Pointer car)
{
    assert(ptr.type == Type_pair);
    Value_pair* raw = VALUE(ptr, Value_pair);
    raw->car = car;
}

void set_cdr(Pointer ptr, Pointer cdr)
{
    assert(ptr.type == Type_pair);
    Value_pair* raw = VALUE(ptr, Value_pair);
    raw->cdr = cdr;
}

Pointer copy(Pointer ptr)
{
    if (ptr.type == Type_nil) {
        return ptr;
    }

    Value_base* base = VALUE(ptr, Value_base);
    if (base->relocated.type == Type_nil) {
        switch (ptr.type) {
            case Type_integer: {
                base->relocated =
                    make_integer(get_integer(ptr));
                break;
            }
            case Type_pair: {
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

Pointer stop_and_copy(Pointer ptr)
{
    int before = allocator_bytesAvailable();

    allocator_swapHeaps();
    ptr = copy(ptr);

    int after = allocator_bytesAvailable();
    fprintf(stderr, "Garbage collected %d -> %d: %d bytes freed\n", before, after, after - before);

    return ptr;
}

void print_value(Pointer ptr)
{
    switch (ptr.type) {
        case Type_nil: {
            printf("nil");
            break;
        }

        case Type_integer: {
            printf("%d", get_integer(ptr));
            break;
        }

        case Type_pair: {
            printf("[");
            print_value(get_car(ptr));
            printf(", ");
            print_value(get_cdr(ptr));
            printf("]");
            break;
        }
    }
}

void print(Pointer ptr)
{
    print_value(ptr);
    printf("\n");
}
