#ifndef headerfilesqrtpricemath
#define headerfilesqrtpricemath

#include "consts.h"
#include "types.h"
#include "util.h"
#include "unsafemath.h"
#include "fullmath.h"

/// @notice Gets the next sqrt price given a delta of token0
/// @dev Always rounds up, because in the exact output case (increasing price) we need to move the price at least
/// far enough to get the desired output amount, and in the exact input case (decreasing price) we need to move the
/// price less in order to not send too much output.
/// The most precise formula for this is liquidity * sqrtPX96 / (liquidity +- amount * sqrtPX96),
/// if this is impossible because of overflow, we calculate liquidity / (liquidity / sqrtPX96 +- amount).
/// @param sqrtPX96 The starting price, i.e. before accounting for the token0 delta
/// @param liquidity The amount of usable liquidity
/// @param amount How much of token0 to add or remove from virtual reserves
/// @param add Whether to add or remove the amount of token0
/// @return The price after adding or removing amount, depending on add
uint160 getNextSqrtPriceFromAmount0RoundingUp(
    uint160 sqrtPX96,
    uint128 liquidity,
    uint256 amount,
    bool add
) {
    // we short circuit amount == 0 because the result is otherwise not guaranteed to equal the input price
    if (amount == 0) return sqrtPX96;
    uint256 numerator1 = uint256(liquidity) << RESOLUTION;

    if (add) {
        uint256 product;
        if ((product = amount * sqrtPX96) / amount == sqrtPX96) {
            uint256 denominator = numerator1 + product;
            if (denominator >= numerator1)
                // always fits in 160 bits
                return mulDivRoundingUp(numerator1, sqrtPX96, denominator);
        }

        return divRoundingUp(numerator1, (numerator1 / sqrtPX96) + amount);
    } else {
        uint256 product;
        // if the product overflows, we know the denominator underflows
        // in addition, we must check that the denominator does not underflow
        require((product = amount * sqrtPX96) / amount == sqrtPX96 && numerator1 > product);
        uint256 denominator = numerator1 - product;
        return mulDivRoundingUp(numerator1, sqrtPX96, denominator);
    }
}

/// @notice Gets the next sqrt price given a delta of token1
/// @dev Always rounds down, because in the exact output case (decreasing price) we need to move the price at least
/// far enough to get the desired output amount, and in the exact input case (increasing price) we need to move the
/// price less in order to not send too much output.
/// The formula we compute is within <1 wei of the lossless version: sqrtPX96 +- amount / liquidity
/// @param sqrtPX96 The starting price, i.e., before accounting for the token1 delta
/// @param liquidity The amount of usable liquidity
/// @param amount How much of token1 to add, or remove, from virtual reserves
/// @param add Whether to add, or remove, the amount of token1
/// @return The price after adding or removing `amount`
uint160 getNextSqrtPriceFromAmount1RoundingDown(
    uint160 sqrtPX96,
    uint128 liquidity,
    uint256 amount,
    bool add
) {
    // if we're adding (subtracting), rounding down requires rounding the quotient down (up)
    // in both cases, avoid a mulDiv for most inputs
    if (add) {
        uint256 quotient = (
            amount <= (uint160(0) - 1)
                ? (amount << RESOLUTION) / liquidity
                : mulDiv(amount, Q96, liquidity)
        );

        return uint256(sqrtPX96) + quotient;
    } else {
        uint256 quotient = (
            amount <= uint160(0) - 1
                ? divRoundingUp(amount << RESOLUTION, liquidity)
                : mulDivRoundingUp(amount, Q96, liquidity)
        );

        require(sqrtPX96 > quotient);
        // always fits 160 bits
        return uint160(sqrtPX96 - quotient);
    }
}

/// @notice Gets the amount0 delta between two prices
/// @dev Calculates liquidity / sqrt(lower) - liquidity / sqrt(upper),
/// i.e. liquidity * (sqrt(upper) - sqrt(lower)) / (sqrt(upper) * sqrt(lower))
/// @param sqrtRatioAX96 A sqrt price
/// @param sqrtRatioBX96 Another sqrt price
/// @param liquidity The amount of usable liquidity
/// @param roundUp Whether to round the amount up or down
/// @return amount0 Amount of token0 required to cover a position of size liquidity between the two passed prices
uint256 getAmount0Delta(
    uint160 sqrtRatioAX96,
    uint160 sqrtRatioBX96,
    uint128 liquidity,
    bool roundUp
) {
    if (sqrtRatioAX96 > sqrtRatioBX96) std::swap(sqrtRatioAX96, sqrtRatioBX96);

    uint256 numerator1 = uint256(liquidity) << RESOLUTION;
    uint256 numerator2 = sqrtRatioBX96 - sqrtRatioAX96;

    require(sqrtRatioAX96 > 0);

    if (roundUp) {
        return divRoundingUp(
            mulDivRoundingUp(numerator1, numerator2, sqrtRatioBX96),
            sqrtRatioAX96
        );
    } else {
        return mulDiv(numerator1, numerator2, sqrtRatioBX96) / sqrtRatioAX96;
    }
}

/// @notice Gets the amount1 delta between two prices
/// @dev Calculates liquidity * (sqrt(upper) - sqrt(lower))
/// @param sqrtRatioAX96 A sqrt price
/// @param sqrtRatioBX96 Another sqrt price
/// @param liquidity The amount of usable liquidity
/// @param roundUp Whether to round the amount up, or down
/// @return amount1 Amount of token1 required to cover a position of size liquidity between the two passed prices
uint256 getAmount1Delta(
    uint160 sqrtRatioAX96,
    uint160 sqrtRatioBX96,
    uint128 liquidity,
    bool roundUp
) {
    if (sqrtRatioAX96 > sqrtRatioBX96) std::swap(sqrtRatioAX96, sqrtRatioBX96);

    if (roundUp) {
        return mulDivRoundingUp(liquidity, sqrtRatioBX96 - sqrtRatioAX96, Q96);
    } else {
        return mulDiv(liquidity, sqrtRatioBX96 - sqrtRatioAX96, Q96);
    }
}

/// @notice Helper that gets signed token0 delta
/// @param sqrtRatioAX96 A sqrt price
/// @param sqrtRatioBX96 Another sqrt price
/// @param liquidity The change in liquidity for which to compute the amount0 delta
/// @return amount0 Amount of token0 corresponding to the passed liquidityDelta between the two prices
int256 getAmount0Delta(
    uint160 sqrtRatioAX96,
    uint160 sqrtRatioBX96,
    int128 liquidity
) {
    if (liquidity < 0) return uint256(0) - getAmount0Delta(sqrtRatioAX96, sqrtRatioBX96, uint128(-liquidity), false);
    return getAmount0Delta(sqrtRatioAX96, sqrtRatioBX96, uint128(liquidity), true);
}

/// @notice Helper that gets signed token1 delta
/// @param sqrtRatioAX96 A sqrt price
/// @param sqrtRatioBX96 Another sqrt price
/// @param liquidity The change in liquidity for which to compute the amount1 delta
/// @return amount1 Amount of token1 corresponding to the passed liquidityDelta between the two prices
int256 getAmount1Delta(
    uint160 sqrtRatioAX96,
    uint160 sqrtRatioBX96,
    int128 liquidity
) {
    return
        liquidity < 0
            ? uint256(0) - getAmount1Delta(sqrtRatioAX96, sqrtRatioBX96, uint128(-liquidity), false)
            : getAmount1Delta(sqrtRatioAX96, sqrtRatioBX96, uint128(liquidity), true);
}

/// @notice Gets the next sqrt price given an input amount of token0 or token1
/// @dev Throws if price or liquidity are 0, or if the next price is out of bounds
/// @param sqrtPX96 The starting price, i.e., before accounting for the input amount
/// @param liquidity The amount of usable liquidity
/// @param amountIn How much of token0, or token1, is being swapped in
/// @param zeroForOne Whether the amount in is token0 or token1
/// @return sqrtQX96 The price after adding the input amount to token0 or token1
uint160 getNextSqrtPriceFromInput(
    uint160 sqrtPX96,
    uint128 liquidity,
    uint256 amountIn,
    bool zeroForOne
) {
    // std::cout << "------?? " << sqrtPX96 << " " << liquidity << " " << amountIn << " " << zeroForOne << std::endl;
    require(sqrtPX96 > 0);
    require(liquidity > 0);

    // round to make sure that we don't pass the target price
    return
        zeroForOne
            ? getNextSqrtPriceFromAmount0RoundingUp(sqrtPX96, liquidity, amountIn, true)
            : getNextSqrtPriceFromAmount1RoundingDown(sqrtPX96, liquidity, amountIn, true);
}

/// @notice Gets the next sqrt price given an output amount of token0 or token1
/// @dev Throws if price or liquidity are 0 or the next price is out of bounds
/// @param sqrtPX96 The starting price before accounting for the output amount
/// @param liquidity The amount of usable liquidity
/// @param amountOut How much of token0, or token1, is being swapped out
/// @param zeroForOne Whether the amount out is token0 or token1
/// @return sqrtQX96 The price after removing the output amount of token0 or token1
uint160 getNextSqrtPriceFromOutput(
    uint160 sqrtPX96,
    uint128 liquidity,
    uint256 amountOut,
    bool zeroForOne
) {
    require(sqrtPX96 > 0);
    require(liquidity > 0);

    // round to make sure that we pass the target price
    return
        zeroForOne
            ? getNextSqrtPriceFromAmount1RoundingDown(sqrtPX96, liquidity, amountOut, false)
            : getNextSqrtPriceFromAmount0RoundingUp(sqrtPX96, liquidity, amountOut, false);
}

#endif