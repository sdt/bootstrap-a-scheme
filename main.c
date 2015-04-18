#include "allocator.h"
#include "types.h"

#include <stdio.h>

#define SIZEOF(type)    \
    printf("sizeof %s = %lu\n", #type, sizeof(type))

static Pointer make_list(int min, int max)
{
    Pointer list = nil_make();
    while (min <= max) {
        list = pair_make(integer_make(max--), list);
    }
    return list;
}

int main(int argc, char* argv[])
{
    allocator_init(16 * 1024);
    Pointer list = make_list(1, 10);
    print(list);

    for (int i = 0; i < 1; i++) {
        Pointer oldList = list;
        list = stopAndCopy(list);
        print(list);
        print(oldList);
    }

    list = integer_make(456);

    print(list);
    Pointer oldList = list;
    list = stopAndCopy(list);
    print(list);
    print(oldList);

    return 0;
}
