#ifndef INCLUDE_UTIL_H
#define INCLUDE_UTIL_H

#define PLURAL(n)   (&"s"[(n)==1])

extern int util_streq(const char* a, const char* b);

#endif // INCLUDE_UTIL_H
