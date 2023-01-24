#ifndef headerfilebitmath
#define headerfilebitmath

#include "types.h"

uint8 mostSignificantBit(uint256 x) {
    // assert(x == "47099173859296676074923436991833339500630638592");
    int r = 0;
    if (x >= uint256("340282366920938463463374607431768211456")) {
        x >>= 128;
        r += 128;
    }
    if (x >= uint256("18446744073709551616")) {
        x >>= 64;
        r += 64;
    }
    if (x >= uint256("4294967296")) {
        x >>= 32;
        r += 32;
    }
    if (x >= uint256("65536")) {
        x >>= 16;
        r += 16;
    }
    if (x >= uint256("256")) {
        x >>= 8;
        r += 8;
    }
    if (x >= uint256("16")) {
        x >>= 4;
        r += 4;
    }
    if (x >= uint256("4")) {
        x >>= 2;
        r += 2;
    }
    if (x >= uint256("2")) r += 1;
    return r;
}

uint8 leastSignificantBit(uint256 x) {
    int r = 255;
    if ((x & (uint128(0)-1)) > 0) {
        r -= 128;
    } else {
        x >>= 128;
    }
    if ((x & (uint64(0)-1)) > 0) {
        r -= 64;
    } else {
        x >>= 64;
    }
    if ((x & (uint32(0)-1)) > 0) {
        r -= 32;
    } else {
        x >>= 32;
    }
    if ((x & uint32((1<<16) - 1)) > 0) {
        r -= 16;
    } else {
        x >>= 16;
    }
    if ((x & uint32((1<<8) - 1)) > 0) {
        r -= 8;
    } else {
        x >>= 8;
    }
    if ((x & uint32((1<<4) - 1)) > 0) {
        r -= 4;
    } else {
        x >>= 4;
    }
    if ((x & uint32(3)) > 0) {
        r -= 2;
    } else {
        x >>= 2;
    }
    if ((x & uint32(1)) > 0) r -= 1;
    return r;
}

#endif