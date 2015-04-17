#ifndef INCLUDE_DEBUG_H
#define INCLUDE_DEBUG_H

#define ASSERT(condition, ...) \
        assert_verbose(condition, __FILE__, __LINE__, __VA_ARGS__)

extern void assert_verbose(int condition, const char* file, int line,
                           const char* fmt, ...);

#endif // INCLUDE_DEBUG_H
