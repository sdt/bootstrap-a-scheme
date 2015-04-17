#include "allocator.h"
#include "types.h"

#include <stdio.h>

#define SIZEOF(type)    \
    printf("sizeof %s = %lu\n", #type, sizeof(type))

int main(int argc, char* argv[])
{
    allocator_init(16 * 1024);
    Pointer list =
        pair_make(integer_make(1),
            pair_make(integer_make(2),
                pair_make(integer_make(3),
                    pair_make(integer_make(4),
                                 nil_make()))));

    for (int i = 0; i < 2; i++) {
        print(list);
        list = stopAndCopy(list);
    }
    print(list);

    list = integer_make(456);

    print(list);
    list = stopAndCopy(list);
    print(list);

    return 0;
}
