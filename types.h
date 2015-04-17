#ifndef INCLUDE_TYPES_H
#define INCLUDE_TYPES_H

#include "forward.h"

/* Use an X-macro to define the types. This list will get reused in a number of
 * places. see: http://en.wikipedia.org/wiki/X_Macro
 */
#define TYPES_XLIST \
    X(nil)          \
    X(integer)      \
    X(pair)         \
    X(boolean)      \
    X(string)       \
    X(symbol)

typedef enum {
    #define X(name) Type_##name,

    TYPES_XLIST

    #undef X

    TYPE_COUNT,
} Type;

typedef struct _Pointer {
    unsigned type   :  3;
    unsigned offset : 29;
} Pointer;

extern Pointer  make_integer(int value);
extern int      get_integer(Pointer ptr);

extern Pointer  make_nil();

extern Pointer  make_pair(Pointer car, Pointer cdr);
extern Pointer  get_car(Pointer ptr);
extern Pointer  get_cdr(Pointer ptr);

extern Pointer  stop_and_copy(Pointer ptr);

extern void print(Pointer ptr);

#endif // INCLUDE_TYPES_H

