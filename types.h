#ifndef INCLUDE_TYPES_H
#define INCLUDE_TYPES_H

#include "forward.h"

typedef enum {
    type_bool,
    type_integer,
    type_nil,
    type_pair,
    type_string,
    type_symbol,

    TYPE_COUNT,
} type;

typedef struct _pointer {
    unsigned type   :  3;
    unsigned offset : 29;
} pointer;

extern pointer  make_integer(Allocator* allocator, int value);
extern int      get_integer(Allocator* allocator, pointer ptr);

extern pointer  make_nil();

extern pointer  make_pair(Allocator* allocator, pointer car, pointer cdr);
extern pointer  get_car(Allocator* allocator, pointer ptr);
extern pointer  get_cdr(Allocator* allocator, pointer ptr);

#endif // INCLUDE_TYPES_H

