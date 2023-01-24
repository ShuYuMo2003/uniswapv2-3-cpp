#ifndef headerfileutil
#define headerfileutil

#include<cstring>

void require(bool condition, std::string msg = "") {
    assert(condition);
}

#endif