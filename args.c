#include "args.h"

#include "exception.h"
#include "types.h"
#include "util.h"
#include "valuestack.h"

int args_count(Pointer args)
{
    int len = 0;
    for ( ; args.type == Type_pair; args = pair_get(args, 1)) {
        len++;
    }
    return len;
}

StackIndex args_extract(const char* caller, Pointer args, int min, int max)
{
    StackIndex before = valuestack_top();

    // First process the mandatory args.
    for (int i = 0; i < min; i++) {
        if (args.type != Type_pair) {
            // We've run out of args.
            THROW("%s: %s%d arg%s expected, %d provided",
                caller, min == max ? "" : "at least ",
                min, PLURAL(min), i);
        }
        PUSH(pair_get(args, 0));
        args = pair_get(args, 1);
    }

    // Then process the optional args, if required.
    for (int i = min; args.type != Type_nil && (i < max); i++) {
        PUSH(pair_get(args, 0));
        args = pair_get(args, 1);
    }

    // Now check that there aren't any more args.
    if (args.type != Type_nil) {
        int got = max + args_count(args);

        THROW("%s: %s%d arg%s expected, %d provided",
              caller, min == max ? "" : "no more than ",
              max, PLURAL(max), got);
    }

    return before;
}

Pointer args_checkType(const char* caller, const char* argName,
                       Pointer argPtr, Type expected)
{
    if (argPtr.type != expected) {
        THROW("%s: %s is %s, expected %s",
            caller, argName, type_name(argPtr.type), type_name(expected));
    }
    return argPtr;
}
