#include "allocator.h"
#include "core.h"
#include "debug.h"
#include "environment.h"
#include "eval.h"
#include "exception.h"
#include "gc.h"
#include "input.h"
#include "reader.h"
#include "symtab.h"
#include "types.h"
#include "valuestack.h"

#include <stdio.h>

int main(int argc, char* argv[])
{
    int heapSize = 16 * 1024 * 1024;

    allocator_init(heapSize);
    valuestack_init(heapSize / 4);
    env_init();
    symtab_init();
    core_init();
    input_init();
    gc_run();

    StackIndex prevTop = valuestack_top();

    char* input;
    while ((input = input_get("bas> ")) != NULL) {
        StackIndex top = valuestack_top();
        ASSERT(top == prevTop, "Value stack out of whack! Was %d, is %d",
            prevTop, top);
        prevTop = top;

        if (setjmp(*exception_buf()) != 0) {
            printf("%s\n", exception_msg());
            valuestack_popTo(top);
            continue;
        }

        StackIndex astIndex = PUSH(readLine(input));
        StackIndex envIndex = PUSH(env_root());

        print(eval(astIndex, envIndex));

        valuestack_drop(2);
    }

    allocator_deinit();
    return 0;
}
