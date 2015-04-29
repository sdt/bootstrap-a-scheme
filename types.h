#ifndef INCLUDE_TYPES_H
#define INCLUDE_TYPES_H

#include "allocator.h"
#include "basetypes.h"

/* Use an X-macro to define the types. This list will get reused in a number of
 * places. see: http://en.wikipedia.org/wiki/X_Macro
 */
#define TYPES_XLIST \
    X(nil)          \
    X(boolean)      \
    X(builtin)      \
    X(integer)      \
    X(lambda)       \
    X(pair)         \
    X(string)       \
    X(symbol)

typedef enum {
    #define X(name) Type_##name,

    TYPES_XLIST

    #undef X

    Type_COUNT,
} Type;

typedef struct _Pointer {
    unsigned type   :  3;
    unsigned offset : 29;
} Pointer;

extern Pointer      boolean_make(int value);
extern int          boolean_get();

extern Pointer      builtin_apply(Pointer ptr, StackIndex argsIndex, StackIndex envIndex);
extern Pointer      builtin_make(int index);

extern Pointer      integer_make(int value);
extern int          integer_get(Pointer ptr);

extern Pointer      lambda_prepareEnv(StackIndex lambdaIndex,
                                      StackIndex argsIndex);
extern Pointer      lambda_make(StackIndex params, StackIndex body,
                                StackIndex env);
extern Pointer      lambda_getBody(Pointer ptr);
extern Pointer      lambda_getParams(Pointer ptr);
extern Pointer      lambda_getEnv(Pointer ptr);

extern Pointer      list_nth(Pointer ptr, int n);

extern Pointer      nil_make();

extern Pointer      pair_make(StackIndex carIndex, StackIndex cdrIndex);
extern Pointer      pair_get(Pointer ptr, int index);
extern void         pair_set(Pointer ptr, int index, Pointer val);

extern Pointer      string_alloc(int length);
extern Pointer      string_make(const char* value);
extern const char*  string_get(Pointer ptr);

extern Pointer      symbol_alloc(int length);
extern Pointer      symbol_make(const char* value);
extern const char*  symbol_get(Pointer ptr);

extern int          pointer_isFalse(Pointer ptr);
extern int          pointer_isTrue(Pointer ptr);
extern Pointer      pointer_move(Pointer ptr);

extern int          type_isObject(Type type);
extern const char*  type_name(int type);
extern void         print(Pointer ptr);

extern unsigned     value_fixup(unsigned offset);
extern int          value_size(Pointer ptr);

#endif // INCLUDE_TYPES_H
