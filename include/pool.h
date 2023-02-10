#ifndef headerfilepool
#define headerfilepool

#include <fstream>
#include <cmath>
#include <cstdlib> // For debug pause only.

#include "consts.h"
#include "global.h"
#include "types.h"
#include "util.h"

#include "swapmath.h"
#include "tick.h"
#include "tick_bitmap.h"
#include "tickmath.h"
#include "liquiditymath.h"

#define LiquidityType typename std::conditional<enable_float, FloatType, uint128>::type
#define PriceType     typename std::conditional<enable_float, FloatType, uint160>::type

template<bool enable_float>
struct Pool{
    uint24 fee;
    int24 tickSpacing;
    LiquidityType maxLiquidityPerTick;
    Slot0<enable_float> slot0;
    LiquidityType liquidity;
    Ticks<enable_float> ticks;
    TickBitMapBaseOnVector tickBitmap; // If some error occurs because of bitmap, just replace the codes below to `TickBitmap tickBitmap;`
    Pool() {}
    Pool(uint24 fee, int24 tickSpacing, LiquidityType maxLiquidityPerTick)
        : fee(fee), tickSpacing(tickSpacing), maxLiquidityPerTick(maxLiquidityPerTick) {
        liquidity = 0;
    }
    Pool(std::string filename) {
        std::ifstream fin(filename);
        fin >> *this;
        fin.close();
    }
    void save(std::string filename) {
        std::ofstream fout(filename);
        fout << *this;
        fout.close();
    }
    void copyFrom(const Pool &o) {
        fee = o.fee, tickSpacing = o.tickSpacing, maxLiquidityPerTick = o.maxLiquidityPerTick;
        slot0 = o.slot0, liquidity = o.liquidity, ticks = o.ticks, tickBitmap = o.tickBitmap;
    }
    const Pool & operator=(const Pool &o) {
        copyFrom(o);
        return *this;
    }
    Pool(const Pool &o) : fee(o.fee), tickSpacing(o.tickSpacing), maxLiquidityPerTick(o.maxLiquidityPerTick) {
        copyFrom(o);
    }
    /// @dev Returns the block timestamp truncated to 32 bits, i.e. mod 2**32. This method is overridden in tests.
    uint32 _blockTimestamp() {
        return uint32(block.timestamp); // truncation is desired
    }
};

template<bool enable_float>
std::istream& operator>>(std::istream& is, Pool<enable_float>& pool) {
    is >> pool.fee >> pool.tickSpacing >> pool.maxLiquidityPerTick
        >> pool.liquidity >> pool.slot0 >> pool.ticks >> pool.tickBitmap;
    return is;
}

template<bool enable_float>
std::ostream& operator<<(std::ostream& os, const Pool<enable_float>& pool) {
    os << pool.fee << " " << pool.tickSpacing << " " << pool.maxLiquidityPerTick << " " << pool.liquidity << std::endl;
    os << pool.slot0 << std::endl;
    os << pool.ticks << pool.tickBitmap;
    return os;
}

/// @inheritdoc IUniswapV3PoolActions
/// @dev not locked because it initializes unlocked
template<bool enable_float>
int24 initialize(Pool<enable_float> *o, PriceType sqrtPriceX96) {
    require(isZero(o->slot0.sqrtPriceX96), "AI");
    int24 tick = getTickAtSqrtRatio(sqrtPriceX96);
    o->slot0 = Slot0<enable_float>(sqrtPriceX96, tick);
    return tick;
}

std::pair<int256, int256> swap(
    Pool<false> * o,
    bool zeroForOne,
    int256 amountSpecified,
    uint160 sqrtPriceLimitX96,
    bool effect)
{
    require(amountSpecified != 0, "AS");

    Slot0<false> slot0Start = o->slot0;

    require(
        zeroForOne
            ? sqrtPriceLimitX96 < slot0Start.sqrtPriceX96 && sqrtPriceLimitX96 > MIN_SQRT_RATIO
            : sqrtPriceLimitX96 > slot0Start.sqrtPriceX96 && sqrtPriceLimitX96 < MAX_SQRT_RATIO,
        "SPL"
    );

    uint128 liquidityCache = o->liquidity;

    bool exactInput = amountSpecified > 0;

    SwapState<false> state = SwapState<false>(
        amountSpecified,
        0,
        slot0Start.sqrtPriceX96,
        slot0Start.tick,
        liquidityCache
    );

    // continue swapping as long as we haven't used the entire input/output and haven't reached the price limit
    while (state.amountSpecifiedRemaining != 0 && state.sqrtPriceX96 != sqrtPriceLimitX96) {
        StepComputations<false> step;
        step.sqrtPriceStartX96 = state.sqrtPriceX96;
        std::tie(step.tickNext, step.initialized) = o->tickBitmap.nextInitializedTickWithinOneWord(
            state.tick,
            o->tickSpacing,
            zeroForOne
        );
        // ensure that we do not overshoot the min/max tick, as the tick bitmap is not aware of these bounds
        if (step.tickNext < MIN_TICK) {
            step.tickNext = MIN_TICK;
        } else if (step.tickNext > MAX_TICK) {
            step.tickNext = MAX_TICK;
        }

        // get the price for the next tick
        step.sqrtPriceNextX96 = getSqrtRatioAtTick<uint160>(step.tickNext);

        std::tie(state.sqrtPriceX96, step.amountIn, step.amountOut, step.feeAmount) = computeSwapStep(
            state.sqrtPriceX96,
            (zeroForOne ? step.sqrtPriceNextX96 < sqrtPriceLimitX96 : step.sqrtPriceNextX96 > sqrtPriceLimitX96)
                ? sqrtPriceLimitX96
                : step.sqrtPriceNextX96,
            state.liquidity,
            state.amountSpecifiedRemaining,
            o->fee
        );

        if (exactInput) {
            state.amountSpecifiedRemaining -= int256((step.amountIn + step.feeAmount));
            state.amountCalculated = state.amountCalculated - int256(step.amountOut);
        } else {
            state.amountSpecifiedRemaining += int256(step.amountOut);
            state.amountCalculated = state.amountCalculated + int256(step.amountIn + step.feeAmount);
        }

        // shift tick if we reached the next price
        if (state.sqrtPriceX96 == step.sqrtPriceNextX96) {
            // if the tick is initialized, run the tick transition
            if (step.initialized) {
                int128 liquidityNet = o->ticks.cross(step.tickNext);
                // safe because liquidityNet cannot be type(int128).min
                if (zeroForOne) liquidityNet = -liquidityNet;
                state.liquidity = addDelta(state.liquidity, liquidityNet);
            }
            state.tick = zeroForOne ? step.tickNext - 1 : step.tickNext;
        } else if (state.sqrtPriceX96 != step.sqrtPriceStartX96) {
            // recompute unless we're on a lower tick boundary (i.e. already transitioned ticks), and haven't moved
            state.tick = getTickAtSqrtRatio(state.sqrtPriceX96);
        }
    }

    if(effect) {
        if (state.tick != slot0Start.tick) {
            o->slot0.sqrtPriceX96 = state.sqrtPriceX96;
            o->slot0.tick = state.tick;
        } else {
            // otherwise just update the price
            o->slot0.sqrtPriceX96 = state.sqrtPriceX96;
        }
        // update liquidity if it changed
        if (liquidityCache != state.liquidity) o->liquidity = state.liquidity;
    }

    int256 amount0, amount1;
    if (zeroForOne == exactInput) {
        amount0 = amountSpecified - state.amountSpecifiedRemaining;
        amount1 = state.amountCalculated;
    } else {
        amount0 = state.amountCalculated;
        amount1 = amountSpecified - state.amountSpecifiedRemaining;
    }
    return std::make_pair(amount0, amount1);
}


std::pair<FloatType, FloatType> swap(
    Pool<true> *o,
    bool zeroForOne,
    FloatType amountSpecified,
    FloatType sqrtPriceLimitX96,
    bool effect)
{

    require(fabs(amountSpecified) > EPS, "AS");

    Slot0<true> slot0Start = o->slot0;

    FloatType liquidityCache = o->liquidity;

    bool exactInput = amountSpecified > 0;

    SwapState<true> state = SwapState<true>(
        amountSpecified,
        0,
        slot0Start.sqrtPriceX96,
        slot0Start.tick,
        liquidityCache
    );

    // continue swapping as long as we haven't used the entire input/output and haven't reached the price limit
    FloatType lastAmountSpecifiedRemaining = 0;

    while ((fabs(lastAmountSpecifiedRemaining - state.amountSpecifiedRemaining) > EPS)
        && fabs(state.amountSpecifiedRemaining) > EPS
        && fabs(state.sqrtPriceX96 - sqrtPriceLimitX96) > EPS) {

        lastAmountSpecifiedRemaining = state.amountSpecifiedRemaining;

        StepComputations<true> step;

        step.sqrtPriceStartX96 = state.sqrtPriceX96;

        std::tie(step.tickNext, step.initialized) = o->tickBitmap.nextInitializedTickWithinOneWord(
            state.tick,
            o->tickSpacing,
            zeroForOne
        );

        // ensure that we do not overshoot the min/max tick, as the tick bitmap is not aware of these bounds
        if (step.tickNext < MIN_TICK) {
            step.tickNext = MIN_TICK;
        } else if (step.tickNext > MAX_TICK) {
            step.tickNext = MAX_TICK;
        }

        // get the price for the next tick
        step.sqrtPriceNextX96 = getSqrtRatioAtTick<FloatType>(step.tickNext);

        std::tie(state.sqrtPriceX96, step.amountIn, step.amountOut, step.feeAmount) = computeSwapStep(
            state.sqrtPriceX96,
            ((zeroForOne ? (step.sqrtPriceNextX96 < sqrtPriceLimitX96) : (step.sqrtPriceNextX96 > sqrtPriceLimitX96))
                ? sqrtPriceLimitX96
                : step.sqrtPriceNextX96),
            state.liquidity,
            state.amountSpecifiedRemaining,
            o->fee
        );

        if (exactInput) {
            state.amountSpecifiedRemaining -= step.amountIn + step.feeAmount;
            state.amountCalculated = state.amountCalculated - step.amountOut;
        } else {
            state.amountSpecifiedRemaining += step.amountOut;
            state.amountCalculated = state.amountCalculated + step.amountIn + step.feeAmount;
        }

        // shift tick if we reached the next price
        if (fabs(state.sqrtPriceX96 - step.sqrtPriceNextX96) < EPS) {
            // if the tick is initialized, run the tick transition
            if (step.initialized) {
                FloatType liquidityNet = o->ticks.cross(step.tickNext);
                // if we're moving leftward, we interpret liquidityNet as the opposite sign
                // safe because liquidityNet cannot be type(int128).min
                if (zeroForOne) liquidityNet = -liquidityNet;
                state.liquidity = addDelta(state.liquidity, liquidityNet);
            }

            state.tick = zeroForOne ? step.tickNext - 1 : step.tickNext;
        } else if (fabs( state.sqrtPriceX96 - step.sqrtPriceStartX96) > EPS) {
            // recompute unless we're on a lower tick boundary (i.e. already transitioned ticks), and haven't moved
            state.tick = getTickAtSqrtRatio(state.sqrtPriceX96);
        }
    }
    if(effect) {
        if (fabs(state.tick - slot0Start.tick) > EPS) {
            o->slot0.sqrtPriceX96 = state.sqrtPriceX96;
            o->slot0.tick = state.tick;
        } else {
            // otherwise just update the price
            o->slot0.sqrtPriceX96 = state.sqrtPriceX96;
        }
        // update liquidity if it changed
        if (liquidityCache != state.liquidity) o->liquidity = state.liquidity;
    }

    FloatType amount0, amount1;
    if (zeroForOne == exactInput) {
        amount0 = amountSpecified - state.amountSpecifiedRemaining;
        amount1 = state.amountCalculated;
    } else {
        amount0 = state.amountCalculated;
        amount1 = amountSpecified - state.amountSpecifiedRemaining;
    }
    // do the transfers and collect payment
    return std::make_pair(amount0, amount1);
}



void checkTicks(int24 tickLower, int24 tickUpper) {
    require(tickLower < tickUpper, "TLU");
    require(tickLower >= MIN_TICK, "TLM");
    require(tickUpper <= MAX_TICK, "TUM");
}

template<bool enable_float>
void _updatePosition(
    Pool<enable_float> * o,
    int24 tickLower,
    int24 tickUpper,
    typename std::conditional<enable_float, FloatType, int128>::type liquidityDelta,
    int24 tick
) {

    // if we need to update the ticks, do it
    bool flippedLower;
    bool flippedUpper;
    if (!isZero(liquidityDelta)) {
        uint32 time = o->_blockTimestamp();
        flippedLower = o->ticks.update(
            tickLower,
            tick,
            liquidityDelta,
            time,
            false,
            o->maxLiquidityPerTick
        );
        flippedUpper = o->ticks.update(
            tickUpper,
            tick,
            liquidityDelta,
            time,
            true,
            o->maxLiquidityPerTick
        );

        if (flippedLower) {
            o->tickBitmap.flipTick(tickLower, o->tickSpacing);
        }
        if (flippedUpper) {
            o->tickBitmap.flipTick(tickUpper, o->tickSpacing);
        }
    }

    // clear any tick data that is no longer needed
    if (liquidityDelta < 0) {
        if (flippedLower) {
            o->ticks.clear(tickLower);
        }
        if (flippedUpper) {
            o->ticks.clear(tickUpper);
        }
    }
}

template<bool enable_float>
typename std::conditional<enable_float, std::tuple<FloatType, FloatType>, std::tuple<int256, int256>>::type  \
_modifyPosition(Pool<enable_float> * o, ModifyPositionParams<enable_float> params) {
    checkTicks(params.tickLower, params.tickUpper);

    Slot0<enable_float> &_slot0 = o->slot0; // SLOAD for gas optimization

    _updatePosition<enable_float>(
        o,
        params.tickLower,
        params.tickUpper,
        params.liquidityDelta,
        _slot0.tick
    );

    typename std::conditional<enable_float, FloatType, int256>::type amount0 = 0, amount1 = 0;

    if (!isZero(params.liquidityDelta)) {
        if (_slot0.tick < params.tickLower) {
            // current tick is below the passed range; liquidity can only become in range by crossing from left to
            // right, when we'll need _more_ token0 (it's becoming more valuable) so user must provide it
            amount0 = getAmount0Delta(
                getSqrtRatioAtTick<PriceType>(params.tickLower),
                getSqrtRatioAtTick<PriceType>(params.tickUpper),
                params.liquidityDelta
            );
        } else if (_slot0.tick < params.tickUpper) {
            // current tick is inside the passed range
            LiquidityType liquidityBefore = o->liquidity; // SLOAD for gas optimization
            amount0 = getAmount0Delta(
                _slot0.sqrtPriceX96,
                getSqrtRatioAtTick<PriceType>(params.tickUpper),
                params.liquidityDelta
            );
            amount1 = getAmount1Delta(
                getSqrtRatioAtTick<PriceType>(params.tickLower),
                _slot0.sqrtPriceX96,
                params.liquidityDelta
            );

            o->liquidity = addDelta(liquidityBefore, params.liquidityDelta);
        } else {
            // current tick is above the passed range; liquidity can only become in range by crossing from right to
            // left, when we'll need _more_ token1 (it's becoming more valuable) so user must provide it
            amount1 = getAmount1Delta(
                getSqrtRatioAtTick<PriceType>(params.tickLower),
                getSqrtRatioAtTick<PriceType>(params.tickUpper),
                params.liquidityDelta
            );
        }
    }
    return std::make_tuple(amount0, amount1);
}



template<bool enable_float>
typename std::conditional<enable_float, std::pair<FloatType, FloatType>, std::pair<uint256, uint256>>::type mint(
    Pool<enable_float> * o,
    address recipient,
    int24 tickLower,
    int24 tickUpper,
    typename std::conditional<enable_float, FloatType, uint128>::type amount,
    bytes32 data
) {
    typedef typename std::conditional<enable_float, FloatType, int256>::type AmountType;
    typedef typename std::conditional<enable_float, FloatType, uint256>::type FinalAmountType;

    require(amount > 0, "AMZ0");
    AmountType amount0Int, amount1Int;
    std::tie(amount0Int, amount1Int) = _modifyPosition<enable_float>(
        o,
        ModifyPositionParams<enable_float>(recipient, tickLower, tickUpper, (AmountType)(amount))
    );

    FinalAmountType amount0 = (FinalAmountType)(amount0Int);
    FinalAmountType amount1 = (FinalAmountType)(amount1Int);
    return std::make_pair(amount0, amount1);
}

template<bool enable_float>
typename std::conditional<enable_float, std::pair<FloatType, FloatType>, std::pair<uint256, uint256>>::type burn(
    Pool<enable_float> * o,
    int24 tickLower,
    int24 tickUpper,
    typename std::conditional<enable_float, FloatType, uint128>::type amount
) {
    typedef typename std::conditional<enable_float, FloatType, int256>::type AmountType;
    typedef typename std::conditional<enable_float, FloatType, uint256>::type FinalAmountType;

    AmountType amount0Int, amount1Int;
    std::tie(amount0Int, amount1Int) = _modifyPosition(
        o,
        ModifyPositionParams<enable_float>(msg.sender, tickLower, tickUpper, -(AmountType)(amount))
    );

    FinalAmountType amount0 = (FinalAmountType)(-amount0Int);
    FinalAmountType amount1 = (FinalAmountType)(-amount1Int);
    return std::make_pair(amount0, amount1);
}



#endif