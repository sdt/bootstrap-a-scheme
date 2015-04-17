#include "allocator.h"
#include "types.h"

#include <stdio.h>

int main(int argc, char* argv[])
{
    Allocator* allocator = allocator_create(16 * 1024 * 1024);
    pointer list =
        make_pair(allocator, make_integer(allocator, 1),
            make_pair(allocator, make_integer(allocator, 2),
                make_pair(allocator, make_integer(allocator, 3),
                    make_pair(allocator, make_integer(allocator, 4),
                                 make_nil()))));

    for (int i = 0; i < 2; i++) {
        print(allocator, list);
        list = stop_and_copy(allocator, list);
    }
    print(allocator, list);

    list = make_integer(allocator, 456);

    print(allocator, list);
    list = stop_and_copy(allocator, list);
    print(allocator, list);

    return 0;
}
