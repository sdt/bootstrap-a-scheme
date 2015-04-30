#ifndef INCLUDE_EXECUTOR_H
#define INCLUDE_EXECUTOR_H

#define EXECUTE_HANDLERS_XLIST \
    X(define)           \
    X(id)               \
    X(var)

/*
#define EXECUTE_HANDLERS_XLIST \
    X(apply)            \
    X(define)           \
    X(id)               \
    X(if)               \
    X(lambda)
*/

typedef enum {
    #define X(name) ExecuteHandler_##name,

    EXECUTE_HANDLERS_XLIST

    #undef X

    ExecuteHandler_COUNT,
} ExecuteHandlerId;

//TODO: untangle this circular dependency
//extern Pointer executor_executeHandler(ExecuteHandlerId handlerId, StackIndex valueIndex, StackIndex envIndex);

#endif // INCLUDE_EXECUTOR_H
