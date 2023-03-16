#ifndef headerfileutil
#define headerfileutil
#include <cmath>
#include <cstring>
#include <cassert>

#include "types.h"
#include "consts.h"

#ifndef NDEBUG
#   define ASSERT(condition, message) \
    do { \
        if (! (condition)) { \
            std::cerr << "Assertion `" #condition "` failed in " << __FILE__ \
                      << " line " << __LINE__ << ": " << message << std::endl; \
            std::terminate(); \
        } \
    } while (false)
#else
#   define ASSERT(condition, message) do { } while (false)
#endif



__attribute__((always_inline)) void require(bool condition, const char * msg) {
    ASSERT(condition, "msg: " << msg);
}

// #define abs(o) ((o) < 0 ? (-(o)) : (o))

template<typename T>
bool isZero(T & o) { return o == 0; }

template<>
bool isZero<FloatType>(FloatType & o) { return fabs(o) < EPS; }


__attribute__((always_inline)) void prefetch_range(void *addr, size_t len)
{
    char *cp;
    char *end = (char*)addr + len;

    for (cp = (char*)addr; cp < end; cp += 64)
        __builtin_prefetch(cp);
}

#endif
