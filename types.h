#ifndef INCLUDE_TYPES_H
#define INCLUDE_TYPES_H

#include "forward.h"

typedef enum {
    type_nil,
    type_integer,
    type_pair,
    type_bool,
    type_string,
    type_symbol,

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

