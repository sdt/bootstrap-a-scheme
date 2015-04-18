#include "allocator.h"
#include "readline.h"
#include "types.h"

#include <stdio.h>

#define SIZEOF(type)    \
    printf("sizeof %s = %lu\n", #type, sizeof(type))

int main(int argc, char* argv[])
{
    allocator_init(1 * 1024);

    const char* input;
    while ((input = readLine("bas> ")) != NULL) {
        printf("%s\n", input);
    }

    return 0;
}
