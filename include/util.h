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



void require(bool condition, std::string msg = "") {
    ASSERT(condition, "msg: " << msg);
}


template<typename T>
bool isZero(T & o) { return o == 0; }

template<>
bool isZero<FloatType>(FloatType & o) { return fabs(o) < EPS; }
#endif
