#include "../include/sqrtpricemath.h"

int main() {
    /*
        tests for getAmount0Delta


        @notice Gets the amount0 delta between two prices
        @dev Calculates liquidity / sqrt(lower) - liquidity / sqrt(upper),
        i.e. liquidity * (sqrt(upper) - sqrt(lower)) / (sqrt(upper) * sqrt(lower))
        @param sqrtRatioAX96 A sqrt price
        @param sqrtRatioBX96 Another sqrt price
        @param liquidity The amount of usable liquidity
        @param roundUp Whether to round the amount up or down
        @return amount0 Amount of token0 required to cover a position of size liquidity between the two passed prices

    */
    assert(getAmount0Delta("1250950067590534638286489891736967", "1264070914265153793583159887015116", "26272715526336817") == "17271696354");

    /*
        tests for getAmount1Delta

        @notice Gets the amount1 delta between two prices
        @dev Calculates liquidity * (sqrt(upper) - sqrt(lower))
        @param sqrtRatioAX96 A sqrt price
        @param sqrtRatioBX96 Another sqrt price
        @param liquidity The amount of usable liquidity
        @param roundUp Whether to round the amount up, or down
        @return amount1 Amount of token1 required to cover a position of size liquidity between the two passed prices

    */
    uint160 sqrtRatioAX96("1248993462782813945679703639744217");
    uint160 sqrtRatioBX96("1267868630852020208767707712718408");
    int128 liquidity("176527367721");
    bool roundUp = true;
    int256 res("42055547280282");
    assert(getAmount1Delta(sqrtRatioAX96, sqrtRatioBX96, liquidity, roundUp) == res);
    assert(getAmount1Delta(sqrtRatioAX96, sqrtRatioBX96, liquidity) == res);
    assert(getAmount1Delta("1234095850533096949679087966753139", "1250950067590534638286489891736967", "26272715526336817") == "5588998105180978119");

    /*
        tests for mulDiv mulDivRoundingUp

        @notice Calculates ceil(a×b÷denominator) with full precision. Throws if result overflows a uint256 or denominator == 0
        @param a The multiplicand
        @param b The multiplier
        @param denominator The divisor
        @return result The 256-bit result
    */
    // a.FromString("176527367721");
    // b.FromString("18875168069206263088004072974191");
    // den.FromString("79228162514264337593543950336");
    // res.FromString("42055547280282");
    // assert(mulDivRoundingUp(a, b, den) == res);
}