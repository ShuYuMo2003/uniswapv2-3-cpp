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

    // std::cerr << "computeSwapStep(sqrtRatioCurrentX96 = " << sqrtRatioCurrentX96.X96ToDouble() << ", sqrtRatioTargetX96 = " << sqrtRatioTargetX96.X96ToDouble() << ", liquidity = " << liquidity << ", amountRemaining = " << amountRemaining << ", feePips = " << feePips << std::endl;

    bool zeroForOne = sqrtRatioCurrentX96 >= sqrtRatioTargetX96;
    bool exactIn = amountRemaining >= 0;

    uint160 sqrtRatioNextX96;
    uint256 amountIn, amountOut, feeAmount;

    // std::cerr << "ARG: " << zeroForOne << " " << exactIn << std::endl;

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


__attribute__((always_inline)) std::tuple<FloatType, FloatType, FloatType> computeSwapStep(
    const FloatType & sqrtRatioCurrentX96,
    const FloatType & sqrtRatioTargetX96,
    const FloatType & liquidity,
    const FloatType & amountRemaining,
    const uint24 & feePips
) {
    // std::cerr << "computeSwapStep(sqrtRatioCurrentX96 = " << sqrtRatioCurrentX96 << ", sqrtRatioTargetX96 = " << sqrtRatioTargetX96 << ", liquidity = " << liquidity << ", amountRemaining = " << amountRemaining << ", feePips = " << feePips << std::endl;

    bool zeroForOne = (sqrtRatioCurrentX96 - sqrtRatioTargetX96 >= -1e-13);


    FloatType sqrtRatioNextX96;
    FloatType amountIn, amountOut;

    // std::cerr << "ARG: " << zeroForOne << " " << exactIn << std::endl;
    const int RoundUpMode = 1;
    const int RoundDownMode = 0;
    bool condition0;

    FloatType amountRemainingLessFee = ((1e6 - feePips) / 1e6) * amountRemaining;// * mulDiv(amountRemaining , 1e6 - feePips, 1e6);
    if(zeroForOne)
        amountIn = getAmount0Delta(sqrtRatioTargetX96, sqrtRatioCurrentX96, liquidity, RoundUpMode);
    else
        amountIn = getAmount1Delta(sqrtRatioCurrentX96, sqrtRatioTargetX96, liquidity, RoundUpMode);

    condition0 = (amountRemainingLessFee >= amountIn);

    if (condition0) sqrtRatioNextX96 = sqrtRatioTargetX96;
    else
        sqrtRatioNextX96 = getNextSqrtPriceFromInput(
            sqrtRatioCurrentX96,
            liquidity,
            amountRemainingLessFee,
            zeroForOne
        );



    // get the input/output amounts
    if(!condition0) {
        if (zeroForOne) {
            amountIn = getAmount0Delta(sqrtRatioNextX96, sqrtRatioCurrentX96, liquidity, RoundUpMode);
            amountOut = getAmount1Delta(sqrtRatioNextX96, sqrtRatioCurrentX96, liquidity, RoundDownMode);
        } else {
            amountIn = getAmount1Delta(sqrtRatioCurrentX96, sqrtRatioNextX96, liquidity, RoundUpMode);
            amountOut = getAmount0Delta(sqrtRatioCurrentX96, sqrtRatioNextX96, liquidity, RoundDownMode);
        }
    } else {
        if (zeroForOne) {
            amountOut = getAmount1Delta(sqrtRatioNextX96, sqrtRatioCurrentX96, liquidity, RoundDownMode);
        } else {
            amountOut = getAmount0Delta(sqrtRatioCurrentX96, sqrtRatioNextX96, liquidity, RoundDownMode);
        }
    }

    if (!condition0) {
        // we didn't reach the target, so take the remainder of the maximum input as fee
        amountIn = amountRemaining;
    } else {
        amountIn = amountIn + ceil(amountIn / ((1e6) - feePips) * feePips);
    }

    return std::make_tuple(sqrtRatioNextX96, amountIn, amountOut);
}

#endif