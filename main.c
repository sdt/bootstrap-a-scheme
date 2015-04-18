#include "allocator.h"
#include "input.h"
#include "reader.h"
#include "types.h"

#include <stdio.h>

#define SIZEOF(type)    \
    printf("sizeof %s = %lu\n", #type, sizeof(type))

int main(int argc, char* argv[])
{
    allocator_init(1 * 1024);

    const char* input;
    while ((input = getInput("bas> ")) != NULL) {
        Pointer list = readLine(input);
        print(list);
    }

    return 0;
}
