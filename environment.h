#ifndef INCLUDE_ENVIRONMENT_H
#define INCLUDE_ENVIRONMENT_H

#include "basetypes.h"
#include "types.h"

extern void         env_init();
extern Pointer      env_root();
extern Pointer      env_make(StackIndex outerIndex);
extern void         env_set(StackIndex envIndex,
                            StackIndex symIndex, StackIndex valIndex);
extern Pointer      env_get(Pointer env, Pointer symbol);

#endif // INCLUDE_ENVIRONMENT_H
