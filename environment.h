#ifndef INCLUDE_ENVIRONMENT_H
#define INCLUDE_ENVIRONMENT_H

#include "basetypes.h"
#include "types.h"

extern void         env_init();
extern StackIndex   env_root();
extern Pointer      env_make(Pointer outer);
extern void         env_set(Pointer env, Pointer symbol, Pointer value);
extern Pointer      env_get(Pointer env, Pointer symbol);

#endif // INCLUDE_ENVIRONMENT_H
