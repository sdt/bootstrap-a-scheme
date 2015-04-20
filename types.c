#include "types.h"

#include "allocator.h"
#include "core.h"
#include "debug.h"
#include "environment.h"
#include "valuestack.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define ALLOC(type)             ((type*)allocateValue(sizeof(type)))
#define DEREF(ptr, type)        ((Value_##type*)getValue(ptr, Type_##type))
#define DEREF_RAW(ptr, type)    ((Value_##type*)getValueRaw(ptr, Type_##type))

typedef struct {
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
    char value[1];  // more space is allocated at the end
} Value_string;

typedef struct {
    Value_base base;
    char value[1];  // more space is allocated at the end
} Value_symbol;

static const char* typeName[] = {
    #define X(name) #name,

    TYPES_XLIST

    #undef X
};

static Pointer nil          = { Type_nil, 0 };
static Pointer false_value  = { Type_boolean, 0 };
static Pointer true_value   = { Type_boolean, 1 };

static StackIndex symbolsIndex = -1;
static StackIndex rootEnvIndex = -1;

void types_init()
{
    rootEnvIndex = valuestack_push(env_make(nil));
    symbolsIndex = valuestack_push(nil);
}

Pointer getRootEnv() { return valuestack_get(rootEnvIndex); }

static void collectGarbage();

const char* type_name(int type)
{
    if ((type >= 0) && (type < Type_COUNT)) {
        return typeName[type];
    }
    return "(invalid type)";
}

Pointer type_check(Pointer ptr, Type expected)
{
    ASSERT(ptr.type == expected, "Expected %s, got %s",
           type_name(expected), type_name(ptr.type));
    return ptr;
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

static Pointer makePointer(Type type, byte* raw)
{
    Pointer ptr = { type, allocator_getOffset(raw) };
    return ptr;
}

static Value_base* allocateValue(int size)
{
    Value_base* base = (Value_base*) allocator_alloc(size);
    if (base == NULL) {
        collectGarbage();
        base = (Value_base*) allocator_alloc(size);
        ASSERT(base != NULL, "Out of memory");
    }
    base->relocated = nil;
    return base;
}

static Value_base* getValueRaw(Pointer ptr, Type type)
{
    type_check(ptr, type);
    return (Value_base*) allocator_getPointer(ptr.offset);
}

static Value_base* getValue(Pointer ptr, Type type)
{
    return getValueRaw(pointer_follow(ptr), type);
}

Pointer pointer_follow(Pointer ptr)
{
    if (!type_isObject(ptr.type)) {
        return ptr;
    }

    Value_base* base = (Value_base*) allocator_getPointer(ptr.offset);
    if (base->relocated.type != Type_nil) {
        printf("Following broken heart for: %s\n", type_name(ptr.type));
        // This value has been moved to the other heap.
        base = (Value_base*) allocator_getPointer(base->relocated.offset);

        // I don't quite see what stops this from happening, but if it does,
        // it's not a good sign.
        ASSERT(base->relocated.type == Type_nil,
            "%s value has moved heaps twice", type_name(ptr.type));
    }
    return makePointer(ptr.type, (byte*) base);
}

Pointer boolean_make(int value)
{
    return value ? true_value : false_value;
}

int boolean_get(Pointer ptr)
{
    type_check(ptr, Type_boolean);
    return ptr.offset;
}

Pointer builtin_make(int offset)
{
    Pointer ptr = { Type_builtin, offset };
    return ptr;
}

Pointer builtin_apply(Pointer ptr, Pointer args, Pointer env)
{
    type_check(ptr, Type_builtin);
    return core_apply(ptr.offset, args, env);
}

Pointer integer_make(int value)
{
    Value_integer* raw = ALLOC(Value_integer);
    raw->value = value;

    return makePointer(Type_integer, (byte*) raw);
}

int integer_get(Pointer ptr)
{
    Value_integer* raw = DEREF(ptr, integer);
    return raw->value;
}

Pointer lambda_apply(Pointer ptr, Pointer args, Pointer env)
{
    Pointer inner = env_make(env);
    Pointer params = lambda_getParams(ptr);

    if (type_isList(params.type)) {
        while (params.type == Type_pair) {
            env_set(inner, pair_get(params, 0), pair_get(args, 0));
            inner = pointer_follow(inner);
            params = pair_get(params, 1);
            args   = pair_get(args, 1);
        }
    }
    else {
        env_set(inner, params, args);
        inner = pointer_follow(inner);
    }
    extern Pointer eval(Pointer, Pointer);
    return eval(lambda_getBody(ptr), inner);
}

Pointer lambda_make(Pointer params, Pointer body, Pointer env)
{
    Value_lambda* raw = ALLOC(Value_lambda);

    // Use pointer_follow here in case we've got a broken heart.
    raw->params = pointer_follow(params);
    raw->body   = pointer_follow(body);
    raw->env    = pointer_follow(env);

    return makePointer(Type_lambda, (byte*) raw);
}

Pointer lambda_getBody(Pointer ptr)
{
    Value_lambda* raw = DEREF(ptr, lambda);
    return raw->body;
}

Pointer lambda_getParams(Pointer ptr)
{
    Value_lambda* raw = DEREF(ptr, lambda);
    return raw->params;
}

Pointer nil_make()
{
    return nil;
}

Pointer pair_make(Pointer car, Pointer cdr)
{
    Value_pair* raw = ALLOC(Value_pair);

    // Use pointer_follow here in case we've got a broken heart.
    raw->value[0] = pointer_follow(car);
    raw->value[1] = pointer_follow(cdr);

    return makePointer(Type_pair, (byte*) raw);
}

Pointer pair_get(Pointer ptr, int index)
{
    ASSERT((index & 1) == index, "Pair index must be 0 or 1, not %d", index);
    Value_pair* raw = DEREF(ptr, pair);
    return raw->value[index];
}

void pair_set(Pointer ptr, int index, Pointer value)
{
    ASSERT((index & 1) == index, "Pair index must be 0 or 1, not %d", index);
    Value_pair* raw = DEREF(ptr, pair);
    raw->value[index] = value;
}

Pointer string_alloc(int length)
{
    int totalSize = sizeof(Value_string) + length;
    return makePointer(Type_string, (byte*) allocateValue(totalSize));
}

const char* string_get(Pointer ptr)
{
    Value_string* raw = DEREF(ptr, string);
    return raw->value;
}

Pointer string_make(const char* s)
{
    Pointer ptr = string_alloc(strlen(s));
    strcpy((char*) string_get(ptr), s);
    return ptr;
}

const char* symbol_get(Pointer ptr)
{
    Value_symbol* raw = DEREF(ptr, symbol);
    return raw->value;
}

Pointer symbol_make(const char* s)
{
    // Check if this symbol already exists.
    for (Pointer ptr = valuestack_get(symbolsIndex);
         ptr.type != Type_nil;
         ptr = pair_get(ptr, 1)) {
        Pointer sym = pair_get(ptr, 0);
        if (strcmp(s, symbol_get(sym)) == 0) {
            return sym;
        }
    }

    // Create a new symbol, and add it to the symbols list.
    Pointer ptr = string_make(s);
    ptr.type = Type_symbol;
    valuestack_set(symbolsIndex, pair_make(ptr, valuestack_get(symbolsIndex)));
    return ptr;
}

Pointer pointer_copy(Pointer ptr)
{
    if (!type_isObject(ptr.type)) {
        return ptr;
    }

    Value_base* base = (Value_base*) allocator_getPointer(ptr.offset);
    if (base->relocated.type == Type_nil) {
        // Be careful to use DEREF_RAW's in here, we don't want to be chasing
        // broken hearts (because they are all broken).
        switch (ptr.type) {
            case Type_integer: {
                Value_integer* old = DEREF_RAW(ptr, integer);
                base->relocated = integer_make(old->value);
                break;
            }
            case Type_lambda: {
                Value_lambda* old = DEREF_RAW(ptr, lambda);
                Value_lambda* new = ALLOC(Value_lambda);
                base->relocated = makePointer(Type_lambda, (byte*) new);

                new->params = pointer_copy(old->params);
                new->body   = pointer_copy(old->body);
                new->env    = pointer_copy(old->env);
                break;
            }
            case Type_pair: {
                Value_pair* old = DEREF_RAW(ptr, pair);
                Value_pair* new = ALLOC(Value_pair);
                base->relocated = makePointer(Type_pair, (byte*) new);

                for (int i = 0; i < 2; i++) {
                    new->value[i] = pointer_copy(old->value[i]);
                }
                break;
            }
            case Type_string: {
                Value_string* old = DEREF_RAW(ptr, string);
                base->relocated = string_make(old->value);
                break;
            }
            case Type_symbol: {
                Value_symbol* old = DEREF_RAW(ptr, symbol);

                // Create the symbol as if it were a string.
                base->relocated = string_make(old->value);
                base->relocated.type = Type_symbol;
                break;
            }
            default: {
                ASSERT(0, "Unexpected %s value", type_name(ptr.type));
                break;
            }
        }
    }
    return base->relocated;
}

static void collectGarbage()
{
    int before = allocator_bytesAvailable();

    allocator_swapHeaps();
    valuestack_swapHeaps();

    int after = allocator_bytesAvailable();
    fprintf(stderr, "Garbage collected %d -> %d: %d bytes freed\n", before, after, after - before);
}

void value_print(Pointer ptr);
void list_print(Pointer ptr)
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

void value_print(Pointer ptr)
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
