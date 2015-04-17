#include "allocator.h"
#include "types.h"

#include <stdio.h>

int main(int argc, char* argv[])
{
    Allocator* alloc = allocator_create(16 * 1024 * 1024);
    pointer list =
        make_pair(alloc, make_integer(alloc, 1),
                  make_pair(alloc, make_integer(alloc, 2),
                       make_pair(alloc, make_integer(alloc, 3),
                                 make_nil())));

    printf("%d\n", get_integer(alloc,
                               get_car(alloc, list)));
    printf("%d\n", get_integer(alloc,
                               get_car(alloc,
                                       get_cdr(alloc, list))));
    return 0;
}
