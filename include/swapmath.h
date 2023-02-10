#ifndef headerfileswapmath
#define headerfileswapmath

#include "types.h"
#include "fullmath.h"
#include "sqrtpricemath.h"

/// @notice Computes the result of swapping some amount in, or amount out, given the parameters of the swap
/// @dev The fee, plus the amount in, will never exceed the amount remaining if the swap's `amountSpecified` is positive
/// @param sqrtRatioCurrentX96 The current sqrt price of the pool
/// @param sqrtRatioTargetX96 The price that cannot be exceeded, from which the direction of the swap is inferred
/// @param liquidity The usable liquidity
/// @param amountRemaining How much input or output amount is remaining to be swapped in/out
/// @param feePips The fee taken from the input amount, expressed in hundredths of a bip
/// @return sqrtRatioNextX96 The price after swapping the amount in/out, not to exceed the price target
/// @return amountIn The amount to be swapped in, of either token0 or token1, based on the direction of the swap
/// @return amountOut The amount to be received, of either token0 or token1, based on the direction of the swap
/// @return feeAmount The amount of input that will be taken as a fee
std::tuple<uint160, uint256, uint256, uint256> computeSwapStep(
    uint160 sqrtRatioCurrentX96,
    uint160 sqrtRatioTargetX96,
    uint128 liquidity,
    int256 amountRemaining,
    uint24 feePips
) {
    bool zeroForOne = sqrtRatioCurrentX96 >= sqrtRatioTargetX96;
    bool exactIn = amountRemaining >= 0;

    uint160 sqrtRatioNextX96;
    uint256 amountIn, amountOut, feeAmount;

    if (exactIn) {
        uint256 amountRemainingLessFee = mulDiv(uint256(amountRemaining) , uint24(1e6) - feePips, uint24(1e6));
        amountIn = zeroForOne
            ? getAmount0Delta(sqrtRatioTargetX96, sqrtRatioCurrentX96, liquidity, true)
            : getAmount1Delta(sqrtRatioCurrentX96, sqrtRatioTargetX96, liquidity, true);
        if (amountRemainingLessFee >= amountIn) sqrtRatioNextX96 = sqrtRatioTargetX96;
        else
            sqrtRatioNextX96 = getNextSqrtPriceFromInput(
                sqrtRatioCurrentX96,
                liquidity,
                amountRemainingLessFee,
                zeroForOne
            );
    } else {
        amountOut = zeroForOne
            ? getAmount1Delta(sqrtRatioTargetX96, sqrtRatioCurrentX96, liquidity, false)
            : getAmount0Delta(sqrtRatioCurrentX96, sqrtRatioTargetX96, liquidity, false);
        if (uint256(-amountRemaining) >= amountOut) sqrtRatioNextX96 = sqrtRatioTargetX96;
        else
            sqrtRatioNextX96 = getNextSqrtPriceFromOutput(
                sqrtRatioCurrentX96,
                liquidity,
                uint256(-amountRemaining),
                zeroForOne
            );
    }

    bool max = sqrtRatioTargetX96 == sqrtRatioNextX96;

    // get the input/output amounts
    if (zeroForOne) {
        amountIn = max && exactIn
            ? amountIn
            : getAmount0Delta(sqrtRatioNextX96, sqrtRatioCurrentX96, liquidity, true);
        amountOut = max && !exactIn
            ? amountOut
            : getAmount1Delta(sqrtRatioNextX96, sqrtRatioCurrentX96, liquidity, false);
    } else {
        amountIn = max && exactIn
            ? amountIn
            : getAmount1Delta(sqrtRatioCurrentX96, sqrtRatioNextX96, liquidity, true);
        amountOut = max && !exactIn
            ? amountOut
            : getAmount0Delta(sqrtRatioCurrentX96, sqrtRatioNextX96, liquidity, false);
    }

    // cap the output amount to not exceed the remaining output amount
    if (!exactIn && amountOut > uint256(-amountRemaining)) {
        amountOut = uint256(-amountRemaining);
    }

    if (exactIn && sqrtRatioNextX96 != sqrtRatioTargetX96) {
        // we didn't reach the target, so take the remainder of the maximum input as fee
        feeAmount = uint256(amountRemaining) - amountIn;
    } else {
        feeAmount = mulDivRoundingUp(amountIn, feePips, uint24(1e6) - feePips);
    }
    return std::make_tuple(sqrtRatioNextX96, amountIn, amountOut, feeAmount);
}


std::tuple<FloatType, FloatType, FloatType, FloatType> computeSwapStep(
    FloatType sqrtRatioCurrentX96,
    FloatType sqrtRatioTargetX96,
    FloatType liquidity,
    FloatType amountRemaining,
    uint24 feePips
) {
    bool zeroForOne = sqrtRatioCurrentX96 >= sqrtRatioTargetX96;
    bool exactIn = amountRemaining >= 0;

    FloatType sqrtRatioNextX96;
    FloatType amountIn, amountOut, feeAmount;

    if (exactIn) {
        FloatType amountRemainingLessFee = mulDiv(amountRemaining , 1e6 - feePips, 1e6);
        amountIn = zeroForOne
            ? getAmount0Delta(sqrtRatioTargetX96, sqrtRatioCurrentX96, liquidity, true)
            : getAmount1Delta(sqrtRatioCurrentX96, sqrtRatioTargetX96, liquidity, true);
        if (amountRemainingLessFee >= amountIn) sqrtRatioNextX96 = sqrtRatioTargetX96;
        else
            sqrtRatioNextX96 = getNextSqrtPriceFromInput(
                sqrtRatioCurrentX96,
                liquidity,
                amountRemainingLessFee,
                zeroForOne
            );
    } else {
        amountOut = zeroForOne
            ? getAmount1Delta(sqrtRatioTargetX96, sqrtRatioCurrentX96, liquidity, false)
            : getAmount0Delta(sqrtRatioCurrentX96, sqrtRatioTargetX96, liquidity, false);
        if (-amountRemaining >= amountOut) sqrtRatioNextX96 = sqrtRatioTargetX96;
        else
            sqrtRatioNextX96 = getNextSqrtPriceFromOutput(
                sqrtRatioCurrentX96,
                liquidity,
                -amountRemaining,
                zeroForOne
            );
    }

    bool max = fabs(sqrtRatioTargetX96 - sqrtRatioNextX96) < EPS;

    // get the input/output amounts
    if (zeroForOne) {
        amountIn = max && exactIn
            ? amountIn
            : getAmount0Delta(sqrtRatioNextX96, sqrtRatioCurrentX96, liquidity, true);
        amountOut = max && !exactIn
            ? amountOut
            : getAmount1Delta(sqrtRatioNextX96, sqrtRatioCurrentX96, liquidity, false);
    } else {
        amountIn = max && exactIn
            ? amountIn
            : getAmount1Delta(sqrtRatioCurrentX96, sqrtRatioNextX96, liquidity, true);
        amountOut = max && !exactIn
            ? amountOut
            : getAmount0Delta(sqrtRatioCurrentX96, sqrtRatioNextX96, liquidity, false);
    }

    // cap the output amount to not exceed the remaining output amount
    if (!exactIn && amountOut > -amountRemaining) {
        amountOut = -amountRemaining;
    }

    if (exactIn && fabs(sqrtRatioNextX96 - sqrtRatioTargetX96) > EPS) {
        // we didn't reach the target, so take the remainder of the maximum input as fee
        feeAmount = amountRemaining - amountIn;
    } else {
        feeAmount = mulDivRoundingUp(amountIn, feePips, (1e6) - feePips);
    }

    return std::make_tuple(sqrtRatioNextX96, amountIn, amountOut, feeAmount);
}

#endif