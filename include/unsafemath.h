#ifndef headerfileunsafemath
#define headerfileunsafemath

#include "types.h"

uint256 divRoundingUp(uint256 x, uint256 y) {
    return x / y + (x % y > 0);
}

#endif