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
    // printf("\nNOW AT computeSwapStep\n");
    // std::cout << "ARGS:\n" << (sqrtRatioCurrentX96 >> 96) << "\n" << (sqrtRatioTargetX96 >> 96) << "\n" << liquidity << "\n" << amountRemaining << "\n";
    bool zeroForOne = sqrtRatioCurrentX96 >= sqrtRatioTargetX96;
    bool exactIn = amountRemaining >= 0;

    uint160 sqrtRatioNextX96;
    uint256 amountIn, amountOut, feeAmount;

    if (exactIn) {
        // std::cout << uint256(amountRemaining) << " " << uint24(1e6) - feePips << " " << uint24(1e6) << std::endl;
        uint256 amountRemainingLessFee = mulDiv(uint256(amountRemaining) , uint24(1e6) - feePips, uint24(1e6));
        // std::cout << amountRemainingLessFee << std::endl;
        // std::cout << zeroForOne << " " << sqrtRatioCurrentX96 << " " << sqrtRatioTargetX96 << " " << liquidity << std::endl;
        amountIn = zeroForOne
            ? getAmount0Delta(sqrtRatioTargetX96, sqrtRatioCurrentX96, liquidity, true)
            : getAmount1Delta(sqrtRatioCurrentX96, sqrtRatioTargetX96, liquidity, true);
        // std::cout << amountIn << std::endl;
        if (amountRemainingLessFee >= amountIn) sqrtRatioNextX96 = sqrtRatioTargetX96;//, puts("CASE 0");
        else
            sqrtRatioNextX96 = getNextSqrtPriceFromInput(
                sqrtRatioCurrentX96,
                liquidity,
                amountRemainingLessFee,
                zeroForOne
            );
        // std::cout << "$1 = " << amountRemainingLessFee << std::endl;
        // std::cout << "$2 = " << amountIn << std::endl;
        // printf("$3 = %.2lf\n", sqrtRatioNextX96.X96ToDouble());
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
    // std::cout << sqrtRatioNextX96 << std::endl;

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
    // puts("");
    return std::make_tuple(sqrtRatioNextX96, amountIn, amountOut, feeAmount);
}


std::tuple<double, double, double, double> computeSwapStep_float(
    double sqrtRatioCurrentX96,
    double sqrtRatioTargetX96,
    double liquidity,
    double amountRemaining,
    uint24 feePips
) {
    // printf("\nNOW AT computeSwapStep\n");
    // printf("ARGS:\n%.20lf\n%.20lf\n%.20lf\n%.20lf\n", sqrtRatioCurrentX96, sqrtRatioTargetX96, liquidity, amountRemaining);
    bool zeroForOne = sqrtRatioCurrentX96 >= sqrtRatioTargetX96;
    bool exactIn = amountRemaining >= 0;

    double sqrtRatioNextX96;
    double amountIn, amountOut, feeAmount;

    if (exactIn) {
        // std::cout << uint256(amountRemaining) << " " << uint24(1e6) - feePips << " " << uint24(1e6) << std::endl;
        double amountRemainingLessFee = mulDiv_float(amountRemaining , 1e6 - feePips, 1e6);
        // std::cout << amountRemainingLessFee << std::endl;
        // std::cout << zeroForOne << " " << sqrtRatioCurrentX96 << " " << sqrtRatioTargetX96 << " " << liquidity << std::endl;
        amountIn = zeroForOne
            ? getAmount0Delta_float(sqrtRatioTargetX96, sqrtRatioCurrentX96, liquidity, true)
            : getAmount1Delta_float(sqrtRatioCurrentX96, sqrtRatioTargetX96, liquidity, true);
        // std::cout << amountIn << std::endl;
        if (amountRemainingLessFee >= amountIn) sqrtRatioNextX96 = sqrtRatioTargetX96;//, puts("CASE 0");
        else
            sqrtRatioNextX96 = getNextSqrtPriceFromInput_float(
                sqrtRatioCurrentX96,
                liquidity,
                amountRemainingLessFee,
                zeroForOne
            );
        // printf("$1 = %.2lf\n", amountRemainingLessFee);
        // printf("$2 = %.2lf\n", amountIn);
        // printf("$3 = %.2lf\n", sqrtRatioNextX96);
    } else {
        amountOut = zeroForOne
            ? getAmount1Delta_float(sqrtRatioTargetX96, sqrtRatioCurrentX96, liquidity, false)
            : getAmount0Delta_float(sqrtRatioCurrentX96, sqrtRatioTargetX96, liquidity, false);
        if (-amountRemaining >= amountOut) sqrtRatioNextX96 = sqrtRatioTargetX96;
        else
            sqrtRatioNextX96 = getNextSqrtPriceFromOutput_float(
                sqrtRatioCurrentX96,
                liquidity,
                -amountRemaining,
                zeroForOne
            );
    }
    // std::cout << sqrtRatioNextX96 << std::endl;

    bool max = fabs(sqrtRatioTargetX96 - sqrtRatioNextX96) < EPS;

    // get the input/output amounts
    if (zeroForOne) {
        amountIn = max && exactIn
            ? amountIn
            : getAmount0Delta_float(sqrtRatioNextX96, sqrtRatioCurrentX96, liquidity, true);
        amountOut = max && !exactIn
            ? amountOut
            : getAmount1Delta_float(sqrtRatioNextX96, sqrtRatioCurrentX96, liquidity, false);
    } else {
        amountIn = max && exactIn
            ? amountIn
            : getAmount1Delta_float(sqrtRatioCurrentX96, sqrtRatioNextX96, liquidity, true);
        amountOut = max && !exactIn
            ? amountOut
            : getAmount0Delta_float(sqrtRatioCurrentX96, sqrtRatioNextX96, liquidity, false);
    }

    // cap the output amount to not exceed the remaining output amount
    if (!exactIn && amountOut > -amountRemaining) {
        amountOut = -amountRemaining;
    }

    if (exactIn && fabs(sqrtRatioNextX96 - sqrtRatioTargetX96) > EPS) {
        // we didn't reach the target, so take the remainder of the maximum input as fee
        feeAmount = amountRemaining - amountIn;
    } else {
        feeAmount = mulDivRoundingUp_float(amountIn, feePips, (1e6) - feePips);
    }

    // puts("");
    return std::make_tuple(sqrtRatioNextX96, amountIn, amountOut, feeAmount);
}

#endif