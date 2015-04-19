#ifndef INCLUDE_TYPES_H
#define INCLUDE_TYPES_H

#include "forward.h"

/* Use an X-macro to define the types. This list will get reused in a number of
 * places. see: http://en.wikipedia.org/wiki/X_Macro
 */
#define TYPES_XLIST \
    X(nil)          \
    X(boolean)      \
    X(builtin)      \
    X(integer)      \
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

extern void         types_init();
extern Pointer      getRootEnv();

extern Pointer      builtin_apply(Pointer ptr, Pointer args, Pointer env);
extern Pointer      builtin_make(int index);

extern Pointer      integer_make(int value);
extern int          integer_get(Pointer ptr);

extern Pointer      nil_make();

extern Pointer      pair_make(Pointer car, Pointer cdr);
extern Pointer      pair_get(Pointer ptr, int index);
extern void         pair_set(Pointer ptr, int index, Pointer val);

extern Pointer      string_make(const char* value);
extern Pointer      string_alloc(int length);
extern const char*  string_get(Pointer ptr);

extern Pointer      symbol_make(const char* value);
extern const char*  symbol_get(Pointer ptr);

extern Pointer      type_check(Pointer ptr, Type expected);
extern int          type_isObject(Type type);
extern const char*  type_name(int type);
extern void         print(Pointer ptr);

#endif // INCLUDE_TYPES_H

