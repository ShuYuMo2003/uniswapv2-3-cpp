#ifndef headerfileliquiditymath
#define headerfileliquiditymath

#include "types.h"
#include "util.h"
#include "consts.h"

/// @notice Add a signed liquidity delta to liquidity and revert if it overflows or underflows
/// @param x The liquidity before change
/// @param y The delta by which liquidity should be changed
/// @return z The liquidity delta
uint128 addDelta(uint128 x, int128 y) {
    uint128 z;
    if (y < 0) {
        require((z = x - uint128(-y)) < x, "LS");
    } else {
        require((z = x + uint128(y)) >= x, "LA");
    }
    return z;
}

double addDelta_float(double x, double y) {
    double z;
    z = x + y;
    require(z > 0 && z <= MAX_UINT128_FLOAT);
    return z;
}

#endif