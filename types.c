#include "types.h"

#include "allocator.h"
#include "args.h"
#include "core.h"
#include "debug.h"
#include "environment.h"
#include "exception.h"
#include "executor.h"
#include "gc.h"
#include "symtab.h"
#include "valuestack.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define ALLOC(type) \
    ((Value_##type*)allocateValue(sizeof(Value_##type), Type_##type))
#define DEREF(ptr, type)        ((Value_##type*)getValue(ptr, Type_##type))

typedef struct {
    // When the object is in the active heap, relocated points to the object.
    // When the object is in the inactive heap, relocated points to the new
    // object.
    // During GC, if this pointer refers to the active heap, the object has
    // already been moved. If it points to the inactive heap, it needs to be
    // moved.
    Pointer relocated;
} Value_base;

typedef struct {
    Value_base base;
    int value;
} Value_integer;

typedef struct {
    Value_base base;
    Pointer params, body, env;
} Value_lambda;

typedef struct {
    Value_base base;
    Pointer value[2];
} Value_pair;

typedef struct {
    Value_base base;
    int size;
    Pointer value[0];
} Value_vector;

typedef struct {
    Value_base base;
    char value[1];  // more space is allocated at the end
} Value_cstring;    // this is used for strings and symbols

typedef struct {
    Value_base base;
    ExecuteHandlerId handlerId;
    Pointer value;  // this is a vector
} Value_executor;

static const char* typeName[] = {
    #define X(name) #name,

    TYPES_XLIST

    #undef X
};

static Pointer nil          = { Type_nil, 0 };
static Pointer false_value  = { Type_boolean, 0 };
static Pointer true_value   = { Type_boolean, 1 };

const char* type_name(int type)
{
    if ((type >= 0) && (type < Type_COUNT)) {
        return typeName[type];
    }
    return "(invalid type)";
}

static Pointer type_assert(Pointer ptr, Type expected)
{
    ASSERT(ptr.type == expected, "Expected %s, got %s",
           type_name(expected), type_name(ptr.type));
    return ptr;
}

void type_init()
{
    ASSERT(Type_COUNT <= (1 << POINTER_TYPE_BITS), "Too many types.");
}

int type_isObject(Type type)
{
    return type != Type_nil
        && type != Type_boolean
        && type != Type_builtin;
}

int type_isList(Type type)
{
    return type == Type_pair || type == Type_nil;
}

int pointer_isFalse(Pointer ptr)
{
    return (ptr.type == Type_boolean) && (ptr.offset == 0);
}

int pointer_isTrue(Pointer ptr)
{
    return !pointer_isFalse(ptr);
}

static Pointer makePointer(Type type, byte* raw)
{
    Pointer ptr = { type, allocator_getOffset(raw) };
    return ptr;
}

static Value_base* allocateValue(int size, Type type)
{
#if DEBUG_GC_EVERY_ALLOC
    gc_run();
#endif
    Value_base* base = (Value_base*) allocator_alloc(size);
    if (base == NULL) {
        gc_run();
        base = (Value_base*) allocator_alloc(size);
        if (base == NULL) {
            valuestack_dump();
        }
        ASSERT(base != NULL,
            "Out of memory allocating %d bytes: %d bytes used, %d bytes free",
                size, allocator_bytesUsed(), allocator_bytesAvailable());
    }
    base->relocated = makePointer(type, (byte*) base);
    return base;
}

static Value_base* getValue(Pointer ptr, Type type)
{
    // Check that the pointer passed is of the expected type.
    type_assert(ptr, type);
    Value_base* base = (Value_base*) allocator_getPointer(ptr.offset);

    // Check that value pointed to is also of the expected type.
    // Something's desperately wrong if this fails.
    type_assert(base->relocated, type);
    return base;
}

Pointer boolean_make(int value)
{
    return value ? true_value : false_value;
}

int boolean_get(Pointer ptr)
{
    type_assert(ptr, Type_boolean);
    return ptr.offset;
}

Pointer builtin_make(int offset)
{
    Pointer ptr = { Type_builtin, offset };
    return ptr;
}

Pointer builtin_apply(Pointer ptr, StackIndex argsIndex, StackIndex envIndex)
{
    type_assert(ptr, Type_builtin);
    return core_apply(ptr.offset, argsIndex, envIndex);
}

Pointer executor_execute(Pointer ptr, StackIndex envIndex)
{
    Value_executor* raw = DEREF(ptr, executor);
    StackIndex valueIndex = PUSH(raw->value);
    //TODO: FIX THIS
extern Pointer executor_executeHandler(ExecuteHandlerId handlerId,
                                StackIndex valueIndex, StackIndex envIndex);
    Pointer ret = executor_executeHandler(raw->handlerId, valueIndex, envIndex);
    POP();
    return ret;
}

Pointer executor_make(ExecuteHandlerId handlerId, StackIndex valueIndex)
{
    Value_executor* raw = ALLOC(executor);
    raw->handlerId = handlerId;
    raw->value    = GET(valueIndex);

    return makePointer(Type_executor, (byte*) raw);
}

ExecuteHandlerId executor_handler(Pointer ptr)
{
    Value_executor* raw = DEREF(ptr, executor);
    return raw->handlerId;
}

Pointer executor_value(Pointer ptr)
{
    Value_executor* raw = DEREF(ptr, executor);
    return raw->value;
}

Pointer integer_make(int value)
{
    Value_integer* raw = ALLOC(integer);
    raw->value = value;

    return makePointer(Type_integer, (byte*) raw);
}

int integer_get(Pointer ptr)
{
    Value_integer* raw = DEREF(ptr, integer);
    return raw->value;
}

Pointer lambda_prepareEnv(StackIndex lambdaIndex, StackIndex argsIndex)
{
    Pointer lambda = GET(lambdaIndex);

    StackIndex paramsIndex = PUSH(lambda_getParams(lambda));
    StackIndex envIndex    = PUSH(lambda_getEnv(lambda));
    StackIndex innerIndex  = PUSH(env_make(envIndex));

    if (type_isList(GET(paramsIndex).type)) {
        args_checkCount("lambda", list_length(GET(paramsIndex)),
                                  list_length(GET(argsIndex)));
        while (GET(paramsIndex).type == Type_pair) {
            StackIndex keyIndex = PUSH(PAIR_GET(paramsIndex, 0));
            StackIndex valIndex = PUSH(PAIR_GET(argsIndex, 0));

            env_set(innerIndex, keyIndex, valIndex);

            SET(paramsIndex, PAIR_GET(paramsIndex, 1));
            SET(argsIndex,   PAIR_GET(argsIndex, 1));
            DROP(2);
        }
    }
    else {
        env_set(innerIndex, paramsIndex, argsIndex);
    }

    Pointer ret = GET(innerIndex);
    DROP(3);
    return ret;
}

Pointer lambda_make(StackIndex paramsIndex, StackIndex bodyIndex,
                    StackIndex envIndex)
{
    Value_lambda* raw = ALLOC(lambda);

    raw->params = GET(paramsIndex);
    raw->body   = GET(bodyIndex);
    raw->env    = GET(envIndex);

    return makePointer(Type_lambda, (byte*) raw);
}

Pointer lambda_getBody(Pointer ptr)
{
    Value_lambda* raw = DEREF(ptr, lambda);
    return raw->body;
}

Pointer lambda_getEnv(Pointer ptr)
{
    Value_lambda* raw = DEREF(ptr, lambda);
    return raw->env;
}

Pointer lambda_getParams(Pointer ptr)
{
    Value_lambda* raw = DEREF(ptr, lambda);
    return raw->params;
}

int list_length(Pointer ptr)
{
    int size = 0;
    while (ptr.type == Type_pair) {
        ptr = pair_get(ptr, 1);
        size++;
    }
    return size;
}

Pointer list_nth(Pointer ptr, int n)
{
    for (int i = 0; i < n; i++) {
        ptr = pair_get(ptr, 1);
    }
    return pair_get(ptr, 0);
}

Pointer nil_make()
{
    return nil;
}

Pointer pair_make(StackIndex carIndex, StackIndex cdrIndex)
{
    Value_pair* raw = ALLOC(pair);

    raw->value[0] = GET(carIndex);
    raw->value[1] = GET(cdrIndex);

    return makePointer(Type_pair, (byte*) raw);
}

Pointer pair_get(Pointer ptr, int index)
{
    ASSERT((index & 1) == index, "Pair index must be 0 or 1, not %d", index);
    Value_pair* raw = DEREF(ptr, pair);
    return raw->value[index];
}

void pair_set(StackIndex ptrIndex, int index, Pointer value)
{
    ASSERT((index & 1) == index, "Pair index must be 0 or 1, not %d", index);
    Value_pair* raw = DEREF(GET(ptrIndex), pair);
    raw->value[index] = value;
}

static Pointer cstring_alloc(int length, Type type)
{
    int totalSize = sizeof(Value_cstring) + length;
    return makePointer(type, (byte*) allocateValue(totalSize, type));
}

static const char* cstring_get(Pointer ptr, Type type)
{
    Value_cstring* raw = (Value_cstring*) getValue(ptr, type);
    return raw->value;
}

static Pointer cstring_make(const char* s, Type type)
{
    Pointer ptr = cstring_alloc(strlen(s), type);
    strcpy((char*) cstring_get(ptr, type), s);
    return ptr;
}

const char* string_get(Pointer ptr)
{
    return cstring_get(ptr, Type_string);
}

Pointer string_alloc(int length)
{
    return cstring_alloc(length, Type_string);
}

Pointer string_make(const char* s)
{
    return cstring_make(s, Type_string);
}

Pointer symbol_alloc(int length)
{
    return cstring_alloc(length, Type_symbol);
}

const char* symbol_get(Pointer ptr)
{
    return cstring_get(ptr, Type_symbol);
}

Pointer symbol_make(const char* s)
{
    // Check if this symbol already exists.
    Pointer ptr = symtab_find(s);

    if (ptr.type == Type_nil) {
        // Create a new symbol, and add it to the symbol table.
        StackIndex symIndex = PUSH(cstring_make(s, Type_symbol));
        symtab_add(symIndex);
        ptr = POP();
    }
    return ptr;
}

Pointer vector_get(Pointer ptr, int index)
{
    Value_vector* vec = DEREF(ptr, vector);
    if ((index < 0) || (index >= vec->size)) {
        THROW("Index %d out of range [0..%d)", index, vec->size);
    }
    return vec->value[index];
}

Pointer vector_set(StackIndex ptrIndex, int index, Pointer value)
{
    Value_vector* vec = DEREF(GET(ptrIndex), vector);
    if ((index < 0) || (index >= vec->size)) {
        THROW("Index %d out of range [0..%d)", index, vec->size);
    }
    return vec->value[index] = value;
}

Pointer vector_make(int size)
{
    int totalSize = sizeof(Value_vector) + size * sizeof(Pointer);
    Value_vector* vec = (Value_vector*) allocateValue(totalSize, Type_vector);
    vec->size = size;
    for (int i = 0; i < size; i++) {
        vec->value[i] = nil_make();
    }
    return makePointer(Type_vector, (byte*) vec);
}

int vector_size(Pointer ptr)
{
    Value_vector* vec = DEREF(ptr, vector);
    return vec->size;
}

Pointer pointer_move(Pointer oldPtr)
{
    // Pointer-only objects never need to be moved.
    if (!type_isObject(oldPtr.type)) {
        return oldPtr;
    }

    // Is the offset is active, this is an already-moved pointer.
    // I need to think about what causes this to happen.
    if (allocator_isOffsetActive(oldPtr.offset)) {
        return oldPtr;
    }

    // Objects in the old heap will point to themselves if they haven't been
    // moved, or to the new heap if they have been moved.
    Value_base* oldBase = getValue(oldPtr, oldPtr.type);
    if (oldBase->relocated.offset != oldPtr.offset) {
        return oldBase->relocated;
    }

    // Copy the old object to the new heap and return the new pointer.
    // Don't fix up any of the value's internal pointers, just relocated.
    // The internal pointers will get fixed up later.
    int size = value_size(oldPtr);
    Value_base* newBase = allocateValue(size, oldPtr.type);

    // allocateValue will have already set up the Value_base struct, we need
    // to copy the rest of the data across.
    memcpy(newBase + 1, oldBase + 1, size - sizeof(Value_base));

    // Finally, set the relocated pointer on the old object to point to the new.
    oldBase->relocated = newBase->relocated;

    return newBase->relocated;
}

unsigned value_fixup(unsigned offset)
{
    Value_base* base = (Value_base*) allocator_getPointer(offset);
    Type type = base->relocated.type;

    switch (type) {
        case Type_pair: {
            Value_pair* pair = (Value_pair*) base;
            for (int i = 0; i < 2; i++) {
                pair->value[i] = pointer_move(pair->value[i]);
            }
        }
        break;

        case Type_lambda: {
            Value_lambda* pair = (Value_lambda*) base;
            pair->params = pointer_move(pair->params);
            pair->body   = pointer_move(pair->body);
            pair->env    = pointer_move(pair->env);
        }
        break;

        case Type_integer:
        case Type_string:
        case Type_symbol:
            // Lets be explicit here so we can catch any screwups.
            break;

        case Type_vector: {
            Value_vector* vec = (Value_vector*) base;
            for (int i = 0; i < vec->size; i++) {
                vec->value[i] = pointer_move(vec->value[i]);
            }
            break;
        }

        case Type_executor: {
            Value_executor* executor = (Value_executor*) base;
            executor->value = pointer_move(executor->value);
            break;
        }

        default:
            ASSERT(0, "Unexpected %s value", type_name(type));
            break;
    }

    return allocator_getOffset(((byte*) base) + value_size(base->relocated));
}


int value_size(Pointer ptr)
{
    if (!type_isObject(ptr.type)) {
        return 0;
    }

    int rawSize = 0;
    switch (ptr.type) {
        case Type_executor:
            rawSize = sizeof(Value_executor);
            break;

        case Type_integer:
            rawSize = sizeof(Value_integer);
            break;

        case Type_lambda:
            rawSize = sizeof(Value_lambda);
            break;

        case Type_pair:
            rawSize = sizeof(Value_pair);
            break;

        case Type_string:
        case Type_symbol:
            // The zero terminator is included in the struct, so we only need
            // to count the string length.
            rawSize = sizeof(Value_cstring)
                    + strlen(cstring_get(ptr, ptr.type));
            break;

        case Type_vector:
            return sizeof(Value_vector) + sizeof(Pointer) * vector_size(ptr);

        default:
            ASSERT(0, "Unexpected %s value", type_name(ptr.type));
            break;
    }
    return allocator_roundUp(rawSize);
}

static void value_print(Pointer ptr);
static void list_print(Pointer ptr)
{
    while (1) {
        // Three cases:
        // (X, nil)  -> "X"
        // (X, pair) -> "X list_print"
        // (X, *)    -> "X . value_print"
        value_print(pair_get(ptr, 0));

        Pointer cdr = pair_get(ptr, 1);
        switch (cdr.type) {
        case Type_nil:
            return;

        case Type_pair:
            putchar(' ');
            ptr = cdr;
            break;

        default:
            printf(" . ");
            value_print(cdr);
            return;
        }
    }
}

static void value_print(Pointer ptr)
{
    switch (ptr.type) {
        case Type_nil: {
            printf("()");
            break;
        }

        case Type_boolean: {
            printf("%s", boolean_get(ptr) ? "true" : "false");
            break;
        }

        case Type_builtin: {
            printf("builtin:%d", ptr.offset);
            break;
        }

        case Type_executor: {
            printf("{X:%d ", executor_handler(ptr));
            value_print(executor_value(ptr));
            putchar('}');
            break;
        }

        case Type_integer: {
            printf("%d", integer_get(ptr));
            break;
        }

        case Type_lambda: {
            printf("lambda:");
            value_print(lambda_getParams(ptr));
            printf("->");
            value_print(lambda_getBody(ptr));
            break;
        }

        case Type_pair: {
            putchar('(');
            list_print(ptr);
            putchar(')');
            break;
        }

        case Type_string: {
            printf("\"%s\"", string_get(ptr));
            break;
        }

        case Type_symbol: {
            printf("%s", symbol_get(ptr));
            break;
        }

        case Type_vector: {
            printf("(vector");
            for (int i = 0, size = vector_size(ptr); i < size; i++) {
                putchar(' ');
                value_print(vector_get(ptr, i));
            }
            putchar(')');
            break;
        }

        default: {
            ASSERT(0, "Unexpected %s value", type_name(ptr.type));
            break;
        }
    }
}

void print(Pointer ptr)
{
    value_print(ptr);
    printf("\n");
}
