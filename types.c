#include "types.h"

#include "allocator.h"
#include "debug.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define ALLOC(type)      ((type*)allocateValue(sizeof(type)))
#define VALUE(ptr, type) ((Value_##type*)getValue(ptr, Type_##type))
#define RAWVAL(ptr, type) ((Value_##type*)getValueRaw(ptr, Type_##type))

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

typedef struct {
    Value_base base;
    char value[1];  // more space is allocated at the end
} Value_string;

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

static Value_base* getValueRaw(Pointer ptr, Type type)
{
    ASSERT(ptr.type == type, "Expected %s, got %s",
           getTypeName(type), getTypeName(ptr.type));

    return (Value_base*) allocator_getPointer(ptr.offset);
}

static Value_base* getValue(Pointer ptr, Type type)
{
    Value_base* base = getValueRaw(ptr, type);
    if (base->relocated.type != Type_nil) {
        // This value has been moved to the other heap.
        base = (Value_base*) allocator_getPointer(base->relocated.offset);

        // I don't quite see what stops this from happening, but if it does,
        // it's not a good sign.
        ASSERT(base->relocated.type == Type_nil,
            "%s value has moved heaps twice", getTypeName(type));
    }
    return base;
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

Pointer string_alloc(int length)
{
    int totalSize = sizeof(Value_string) + length;
    return makePointer(Type_string, (byte*) allocateValue(totalSize));
}

const char* string_get(Pointer ptr)
{
    Value_string* raw = VALUE(ptr, string);
    return raw->value;
}

Pointer string_make(const char* s)
{
    Pointer ptr = string_alloc(strlen(s));
    strcpy((char*) string_get(ptr), s);
    return ptr;
}

Pointer copy(Pointer ptr)
{
    if (ptr.type == Type_nil) {
        return ptr;
    }

    Value_base* base = (Value_base*) allocator_getPointer(ptr.offset);
    if (base->relocated.type == Type_nil) {
        // Be careful to copy RAWVAL's in here, we don't want to be chasing
        // broken hearts.
        switch (ptr.type) {
            case Type_integer: {
                Value_integer* raw = RAWVAL(ptr, integer);
                base->relocated = integer_make(raw->value);
                break;
            }
            case Type_pair: {
                base->relocated = pair_make(nil, nil);
                Value_pair* raw = RAWVAL(ptr, pair);
                for (int i = 0; i < 2; i++) {
                    pair_set(base->relocated, i, copy(raw->value[i]));
                }
                break;
            }
            case Type_string: {
                Value_string* raw = RAWVAL(ptr, string);
                base->relocated = string_make(raw->value);
                break;
            }
            default: {
                ASSERT(0, "Unexpected %s value", getTypeName(ptr.type));
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

        case Type_string: {
            printf("\"%s\"", string_get(ptr));
            break;
        }

        default: {
            ASSERT(0, "Unexpected %s value", getTypeName(ptr.type));
            break;
        }
    }
}

void print(Pointer ptr)
{
    value_print(ptr);
    printf("\n");
}
