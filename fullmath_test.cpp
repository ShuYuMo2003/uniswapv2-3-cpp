#include "fullmath.h"

int main() {
    /*
        tests for mulDiv

        @notice Calculates floor(a×b÷denominator) with full precision. Throws if result overflows a uint256 or denominator == 0
        @param a The multiplicand
        @param b The multiplier
        @param denominator The divisor
        @return result The 256-bit result
        @dev Credit to Remco Bloemen under MIT license https://xn--2-umb.com/21/muldiv

    */
    uint256 a("176527367721");
    uint256 b("18875168069206263088004072974191");
    uint256 den("79228162514264337593543950336");
    uint256 res("42055547280281");
    assert(mulDiv(a, b, den) == res);

    /*
        tests for mulDiv mulDivRoundingUp

        @notice Calculates ceil(a×b÷denominator) with full precision. Throws if result overflows a uint256 or denominator == 0
        @param a The multiplicand
        @param b The multiplier
        @param denominator The divisor
        @return result The 256-bit result
    */
    a.FromString("176527367721");
    b.FromString("18875168069206263088004072974191");
    den.FromString("79228162514264337593543950336");
    res.FromString("42055547280282");
    assert(mulDivRoundingUp(a, b, den) == res);
}