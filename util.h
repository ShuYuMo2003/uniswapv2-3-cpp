#ifndef headerfileutil
#define headerfileutil

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

#include<cstring>
#include <cassert>

void require(bool condition, std::string msg = "") {
    ASSERT(condition, "msg: " << msg);
}

#endif