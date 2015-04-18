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

    Type_COUNT,
} Type;

typedef struct _Pointer {
    unsigned type   :  3;
    unsigned offset : 29;
} Pointer;

extern Pointer  integer_make(int value);
extern int      integer_get(Pointer ptr);

extern Pointer  nil_make();

extern Pointer  pair_make(Pointer car, Pointer cdr);
extern Pointer  pair_get(Pointer ptr, int index);

extern Pointer  stopAndCopy(Pointer ptr);

extern void     print(Pointer ptr);

#endif // INCLUDE_TYPES_H

