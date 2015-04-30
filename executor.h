#ifndef INCLUDE_EXECUTOR_H
#define INCLUDE_EXECUTOR_H

#define EXECUTE_HANDLERS_XLIST \
    X(apply)            \
    X(define)           \
    X(id)               \
    X(if)               \
    X(lambda)           \
    X(vector)

typedef enum {
    #define X(name) ExecuteHandler_##name,

    EXECUTE_HANDLERS_XLIST

    #undef X

    ExecuteHandler_COUNT,
} ExecuteHandlerId;

#endif // INCLUDE_EXECUTOR_H
