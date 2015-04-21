#include "types.h"

#include "allocator.h"
#include "core.h"
#include "debug.h"
#include "environment.h"
#include "gc.h"
#include "symtab.h"
#include "valuestack.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define ALLOC(type)             ((type*)allocateValue(sizeof(type)))
#define DEREF(ptr, type)        ((Value_##type*)getValue(ptr, Type_##type))

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

int pointer_isTrue(Pointer ptr)
{
    return !pointer_isFalse(ptr);
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
        gc_run();
        base = (Value_base*) allocator_alloc(size);
        ASSERT(base != NULL, "Out of memory");
    }
    base->relocated = nil;
    return base;
}

static Value_base* getValue(Pointer ptr, Type type)
{
    type_check(ptr, type);
    return (Value_base*) allocator_getPointer(ptr.offset);
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

Pointer builtin_apply(Pointer ptr, StackIndex argsIndex, StackIndex envIndex)
{
    type_check(ptr, Type_builtin);
    return core_apply(ptr.offset, argsIndex, envIndex);
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

Pointer lambda_apply(StackIndex lambdaIndex, StackIndex argsIndex,
                    StackIndex envIndex)
{
    StackIndex innerIndex  = PUSH(env_make(envIndex));
    StackIndex paramsIndex = PUSH(lambda_getParams(GET(lambdaIndex)));

    if (type_isList(GET(paramsIndex).type)) {
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

    StackIndex bodyIndex = PUSH(lambda_getBody(GET(lambdaIndex)));

    extern Pointer eval(StackIndex, StackIndex);
    Pointer ret = eval(bodyIndex, innerIndex);

    DROP(3);

    return ret;
}

Pointer lambda_make(StackIndex paramsIndex, StackIndex bodyIndex,
                    StackIndex envIndex)
{
    Value_lambda* raw = ALLOC(Value_lambda);

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

Pointer lambda_getParams(Pointer ptr)
{
    Value_lambda* raw = DEREF(ptr, lambda);
    return raw->params;
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
    Value_pair* raw = ALLOC(Value_pair);

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
    Pointer ptr = symtab_find(s);

    if (ptr.type == Type_nil) {
        // Create a new symbol, and add it to the symbol table.
        StackIndex symIndex = PUSH(string_make(s));
        symtab_add(symIndex);
        ptr = POP();
    }
    return ptr;
}

Pointer pointer_copy(Pointer ptr)
{
    if (!type_isObject(ptr.type)) {
        return ptr;
    }

    Value_base* base = (Value_base*) allocator_getPointer(ptr.offset);
    if (base->relocated.type == Type_nil) {
        switch (ptr.type) {
            case Type_integer: {
                Value_integer* old = DEREF(ptr, integer);
                base->relocated = integer_make(old->value);
                break;
            }
            case Type_lambda: {
                Value_lambda* old = DEREF(ptr, lambda);
                Value_lambda* new = ALLOC(Value_lambda);
                base->relocated = makePointer(Type_lambda, (byte*) new);

                new->params = pointer_copy(old->params);
                new->body   = pointer_copy(old->body);
                new->env    = pointer_copy(old->env);
                break;
            }
            case Type_pair: {
                Value_pair* old = DEREF(ptr, pair);
                Value_pair* new = ALLOC(Value_pair);
                base->relocated = makePointer(Type_pair, (byte*) new);

                for (int i = 0; i < 2; i++) {
                    new->value[i] = pointer_copy(old->value[i]);
                }
                break;
            }
            case Type_string: {
                Value_string* old = DEREF(ptr, string);
                base->relocated = string_make(old->value);
                break;
            }
            case Type_symbol: {
                Value_symbol* old = DEREF(ptr, symbol);

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
