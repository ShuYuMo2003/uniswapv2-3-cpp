#ifndef headerfileutil
#define headerfileutil

#include<cstring>
#include <cassert>

void require(bool condition, std::string msg = "") {
    assert(condition);
}

#endif