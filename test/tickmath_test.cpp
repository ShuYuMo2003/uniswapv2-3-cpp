#include "../include/tickmath.h"

int main() {
    /*
        tests for getSqrtRatioAtTick

        @notice Calculates sqrt(1.0001^tick) * 2^96
        @dev Throws if |tick| > max tick
        @param tick The input tick for the above formula
        @return sqrtPriceX96 A Fixed point Q64.96 number representing the sqrt of the ratio of the two assets (token1/token0)
        at the given tick
    */
    uint160 x;
    x.FromString("1269771765637413962888300459067027");
    assert(getSqrtRatioAtTick(193650) == x);
    x.FromString("1248993462782813945679703639744217");
    assert(getSqrtRatioAtTick(193320) == x);
    x.FromString("1267868630852020208767707712718408");
    assert(getSqrtRatioAtTick(193620) == x);
    x.FromString("1266031645321379351608470426016318");
    assert(getSqrtRatioAtTick(193591) == x);

    /*
        tests for getTickAtSqrtRatio

        @notice Calculates the greatest tick value such that getRatioAtTick(tick) <= ratio
        @dev Throws in case sqrtPriceX96 < MIN_SQRT_RATIO, as MIN_SQRT_RATIO is the lowest value getRatioAtTick may
        ever return.
        @param sqrtPriceX96 The sqrt ratio for which to compute the tick as a Q64.96
        @return tick The greatest tick for which the ratio is less than or equal to the input ratio
    */
    x.FromString("1269762490691099769766804172198177");
    assert(getTickAtSqrtRatio(x) == 193649);
}