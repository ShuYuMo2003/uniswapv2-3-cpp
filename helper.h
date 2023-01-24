#ifndef headerfilehelper
#define headerfilehelper

#include "types.h"
typedef ttmath::UInt<512> uint512;
uint256 mulmod(uint256 a, uint256 b, uint256 denominator) {
    uint512 r = a; r *= b;
    return r % denominator;
}

#endif