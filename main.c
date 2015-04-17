#include "allocator.h"
#include "types.h"

#include <stdio.h>

#define SIZEOF(type)    \
    printf("sizeof %s = %lu\n", #type, sizeof(type))

int main(int argc, char* argv[])
{
    allocator_init(16 * 1024);
    Pointer list =
        make_pair(make_integer(1),
            make_pair(make_integer(2),
                make_pair(make_integer(3),
                    make_pair(make_integer(4),
                                 make_nil()))));

    for (int i = 0; i < 2; i++) {
        print(list);
        list = stop_and_copy(list);
    }
    print(list);

    list = make_integer(456);

    print(list);
    list = stop_and_copy(list);
    print(list);

    return 0;
}
