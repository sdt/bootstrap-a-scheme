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
    #define X(name) type_##name,

    TYPES_XLIST

    #undef X

    TYPE_COUNT,
} type;

typedef struct _pointer {
    unsigned type   :  3;
    unsigned offset : 29;
} pointer;

extern pointer  make_integer(int value);
extern int      get_integer(pointer ptr);

extern pointer  make_nil();

extern pointer  make_pair(pointer car, pointer cdr);
extern pointer  get_car(pointer ptr);
extern pointer  get_cdr(pointer ptr);

extern pointer  stop_and_copy(pointer ptr);

extern void print(pointer ptr);

#endif // INCLUDE_TYPES_H

