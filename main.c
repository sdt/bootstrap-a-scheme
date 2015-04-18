#include "allocator.h"
#include "types.h"

#include <stdio.h>

#define SIZEOF(type)    \
    printf("sizeof %s = %lu\n", #type, sizeof(type))

static Pointer list_make(int min, int max)
{
    Pointer list = nil_make();
    while (min <= max) {
        list = pair_make(integer_make(max--), list);
    }
    return list;
}

int main(int argc, char* argv[])
{
    allocator_init(1 * 1024);

    for (int x = 0; x < 1000; x++) {
        root_set(pair_make(integer_make(x), root_get()));
        integer_make(x);
        print(root_get());
    }

    return 0;
}
