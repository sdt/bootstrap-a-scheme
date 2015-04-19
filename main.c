#include "allocator.h"
#include "exception.h"
#include "input.h"
#include "reader.h"
#include "types.h"

#include <stdio.h>

int main(int argc, char* argv[])
{
    allocator_init(1 * 1024);

    char* input;
    while ((input = getInput("bas> ")) != NULL) {
        EXCEPTION_SCOPE;

        Pointer list = readLine(input);
        print(list);
    }

    return 0;
}
