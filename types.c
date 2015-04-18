#include "types.h"

#include "allocator.h"
#include "debug.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#define ALLOC(type)      ((type*)allocateValue(sizeof(type)))
#define VALUE(ptr, type) ((Value_##type*)getValue(ptr, Type_##type))

typedef struct {
    Pointer relocated;
} Value_base;

typedef struct {
    Value_base base;
    int value;
} Value_integer;

typedef struct {
    Value_base base;
    Pointer value[2];
} Value_pair;

static const char* typeName[] = {
    #define X(name) #name,

    TYPES_XLIST

    #undef X
};

static Pointer nil = { Type_nil, 0 };

static const char* getTypeName(int type) {
    if ((type >= 0) && (type < Type_COUNT)) {
        return typeName[type];
    }
    return "(invalid type)";
}

static Pointer makePointer(Type type, byte* raw)
{
    Pointer ptr = { type, allocator_getOffset(raw) };
    return ptr;
}

static Value_base* allocateValue(int size)
{
    Value_base* base = (Value_base*) allocator_alloc(size);
    base->relocated = nil;
    return base;
}

static Value_base* getValue(Pointer ptr, Type type)
{
    ASSERT(ptr.type == type, "Expected %s, got %s",
           getTypeName(type), getTypeName(ptr.type));

    return (Value_base*) allocator_getPointer(ptr.offset);
}

Pointer integer_make(int value)
{
    Value_integer* raw = ALLOC(Value_integer);
    raw->value = value;

    return makePointer(Type_integer, (byte*) raw);
}

int integer_get(Pointer ptr)
{
    Value_integer* raw = VALUE(ptr, integer);
    return raw->value;
}

Pointer nil_make()
{
    return nil;
}

Pointer pair_make(Pointer car, Pointer cdr)
{
    Value_pair* raw = ALLOC(Value_pair);
    raw->value[0] = car;
    raw->value[1] = cdr;

    return makePointer(Type_pair, (byte*) raw);
}

Pointer pair_get(Pointer ptr, int index)
{
    ASSERT((index & 1) == index, "Pair index must be 0 or 1, not %d", index);
    Value_pair* raw = VALUE(ptr, pair);
    return raw->value[index];
}

void pair_set(Pointer ptr, int index, Pointer value)
{
    ASSERT((index & 1) == index, "Pair index must be 0 or 1, not %d", index);
    Value_pair* raw = VALUE(ptr, pair);
    raw->value[index] = value;
}

Pointer copy(Pointer ptr)
{
    if (ptr.type == Type_nil) {
        return ptr;
    }

    Value_base* base = (Value_base*) allocator_getPointer(ptr.offset);
    if (base->relocated.type == Type_nil) {
        switch (ptr.type) {
            case Type_integer: {
                base->relocated =
                    integer_make(integer_get(ptr));
                break;
            }
            case Type_pair: {
                base->relocated = pair_make(nil, nil);
                for (int i = 0; i < 2; i++) {
                    pair_set(base->relocated, i, copy(pair_get(ptr, i)));
                }
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

Pointer stopAndCopy(Pointer ptr)
{
    int before = allocator_bytesAvailable();

    allocator_swapHeaps();
    ptr = copy(ptr);

    int after = allocator_bytesAvailable();
    fprintf(stderr, "Garbage collected %d -> %d: %d bytes freed\n", before, after, after - before);

    return ptr;
}

void value_print(Pointer ptr)
{
    switch (ptr.type) {
        case Type_nil: {
            printf("nil");
            break;
        }

        case Type_integer: {
            printf("%d", integer_get(ptr));
            break;
        }

        case Type_pair: {
            printf("[");
            value_print(pair_get(ptr, 0));
            printf(", ");
            value_print(pair_get(ptr, 1));
            printf("]");
            break;
        }
    }
}

void print(Pointer ptr)
{
    value_print(ptr);
    printf("\n");
}
