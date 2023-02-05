#ifndef headerfilefullmath
#define headerfilefullmath
#include <cmath>
#include "helper.h"
#include "types.h"
#include "util.h"

/// @notice Calculates floor(a×b÷denominator) with full precision. Throws if result overflows a uint256 or denominator == 0
/// @param a The multiplicand
/// @param b The multiplier
/// @param denominator The divisor
/// @return result The 256-bit result
/// @dev Credit to Remco Bloemen under MIT license https://xn--2-umb.com/21/muldiv
uint256 mulDiv(uint256 a, uint256 b, uint256 denominator) {
    uint512 _a = a, _b = b;
    return _a * _b / denominator;
}

double mulDiv_float(double a, double b, double denominator) {
    long double _a = a, _b = b;
    return floor(_a * _b / denominator);
}

/// @notice Calculates ceil(a×b÷denominator) with full precision. Throws if result overflows a uint256 or denominator == 0
/// @param a The multiplicand
/// @param b The multiplier
/// @param denominator The divisor
/// @return result The 256-bit result
uint256 mulDivRoundingUp(
    uint256 a,
    uint256 b,
    uint256 denominator
) {
    uint256 result = mulDiv(a, b, denominator);
    if (mulmod(a, b, denominator) > 0) {
        require(result < uint256(0) - 1, "F36");
        result++;
    }
    return result;
}

double mulDivRoundingUp_float(
    double a,
    double b,
    double denominator
) {
    long double _a = a, _b = b;
    return (_a * _b / denominator);
}

#endif