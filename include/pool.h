#ifndef headerfilepool
#define headerfilepool

#include <fstream>
#include <cmath>
#include <cstdlib> // For debug pause only.

#include "types.h"
#include "consts.h"
#include "util.h"

#include "swapmath.h"
#include "ticks.h"
#include "_tick.h"
#include "tickmath.h"
#include "liquiditymath.h"

#define LiquidityType typename std::conditional<enable_float, FloatType, uint128>::type
#define PriceType     typename std::conditional<enable_float, FloatType, uint160>::type


template<bool enable_float>
struct StepComputations {
    // the price at the beginning of the step
    typename std::conditional<enable_float, FloatType, uint160>::type sqrtPriceStartX96;
    // the next tick to swap to from the current tick in the swap direction
    _Tick<enable_float> * tickNext;
    // whether tickNext is initialized or not
    bool initialized;
    // sqrt(price) for the next tick (1/0)
    typename std::conditional<enable_float, FloatType, uint160>::type sqrtPriceNextX96;
    // how much is being swapped in in this step
    typename std::conditional<enable_float, FloatType, uint256>::type amountIn;
    // how much is being swapped out
    typename std::conditional<enable_float, FloatType, uint256>::type amountOut;
    // how much fee is being paid in
    typename std::conditional<enable_float, FloatType, uint256>::type feeAmount;
};


template<bool enable_float>
struct Pool{
    bool poolType;
    uint24 fee;
    int24 tickSpacing;
    LiquidityType maxLiquidityPerTick;
    Slot0<enable_float> slot0;
    LiquidityType liquidity;
    Ticks<enable_float> ticks;
    Pool() { poolType = enable_float; }
    Pool(uint24 fee, int24 tickSpacing, LiquidityType maxLiquidityPerTick)
        : fee(fee), tickSpacing(tickSpacing), maxLiquidityPerTick(maxLiquidityPerTick) {
        liquidity = 0;
        poolType = enable_float;
    }
    const Pool & operator=(const Pool &o) {
        assert(("Don't use default copy on pool, You should use `CopyPool()` to do so.", false));
    }
};

template<bool enable_float>
char * fetchLowerAddress(Pool<enable_float> * o) {
    return (char *)o;
}

template<bool enable_float> // Next address of the last address of the pool.
char * fetchUpperAddress(Pool<enable_float> * o) {
    return (char *)((&(o->ticks.temp)) + (1 + o->ticks.length));
}

template<bool enable_float>
size_t sizeOfPool(Pool<enable_float> * o) {
    return fetchUpperAddress(o) - fetchLowerAddress(o);
}

template<bool enable_float>
size_t CopyPool(Pool<enable_float> * from, Pool<enable_float> * to) {
    // assert(from->poolType == enable_float && to->poolType == enable_float);

    char * lowerAddress = fetchLowerAddress(from);
    char * UpperAddress = fetchUpperAddress(from);

    memcpy((void *)to, from, UpperAddress - lowerAddress);
    return UpperAddress - lowerAddress;
}

template<bool enable_float>
void SavePool(Pool<enable_float> * o, std::string filename) {
    char * lowerAddress = fetchLowerAddress(o);
    char * UpperAddress = fetchUpperAddress(o);
    size_t size = UpperAddress - lowerAddress;

    FILE * fptr = fopen(filename.c_str(), "wb");
    assert(fptr != NULL);
    fwrite(o, 1, size, fptr);
    fclose(fptr);
}

template<bool enable_float>
void LoadPool(Pool<enable_float> * o, std::string filename) {
    static char buffer[1024 * 1024];

    FILE * fptr = fopen(filename.c_str(), "rb");

    assert(fptr != NULL);
    fread(buffer, 1024, 1024, fptr);
    fclose(fptr);

    CopyPool((Pool<enable_float> *)buffer, o);
}

void GenerateFloatPool(const Pool<false> * from, Pool<true> * to) {
    to->poolType            = true;
    to->fee                 = from->fee;
    to->tickSpacing         = from->tickSpacing;
    to->maxLiquidityPerTick = from->maxLiquidityPerTick.ToDouble();
    to->slot0.sqrtPriceX96  = from->slot0.sqrtPriceX96.X96ToDouble();
    to->slot0.tick          = from->slot0.tick;
    to->liquidity           = from->liquidity.ToDouble();


    to->ticks.length = from->ticks.length;
    _Tick<true> * now = (&to->ticks.temp) + 1;

    _Tick<false> * beginPtr = ((_Tick<false> *)(&from->ticks.temp)) + 1;
    _Tick<false> * endPtr   = beginPtr + from->ticks.length;
    for(_Tick<false> * i = beginPtr; i < endPtr; i++) {
        now->id             = i->id;
        now->liquidityGross = i->liquidityGross.ToDouble();
        now->liquidityNet   = i->liquidityNet.ToDouble();
        now++;
    }
}

template<bool enable_float>
std::istream& operator>>(std::istream& is, Pool<enable_float>& pool) {
    is >> pool.fee >> pool.tickSpacing >> pool.maxLiquidityPerTick
        >> pool.liquidity >> pool.slot0 >> pool.ticks;
    return is;
}

template<bool enable_float>
std::ostream& operator<<(std::ostream& os, const Pool<enable_float>& pool) {
    os << pool.fee << " " << pool.tickSpacing << " " << pool.maxLiquidityPerTick << " " << pool.liquidity << std::endl;
    os << pool.slot0 << std::endl;
    os << pool.ticks;
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
    bool newOperation = true;
    // std::cerr << "at the beginning of swap " << std::endl;
    // o->slot0.print();

    require(amountSpecified != 0, "AS");

    Slot0<false> slot0Start = o->slot0;

    // std::cerr << "check" << std::endl;
    // std::cerr << zeroForOne << std::endl;
    // std::cerr << sqrtPriceLimitX96 << " " << slot0Start.sqrtPriceX96 << std::endl;
    // std::cerr << "Range [" << MIN_SQRT_RATIO << ", " << MAX_SQRT_RATIO << "]" << std::endl;
    // require(
    //     zeroForOne
    //         ? sqrtPriceLimitX96 < slot0Start.sqrtPriceX96 && sqrtPriceLimitX96 > MIN_SQRT_RATIO
    //         : sqrtPriceLimitX96 > slot0Start.sqrtPriceX96 && sqrtPriceLimitX96 < MAX_SQRT_RATIO,
    //     "SPL"
    // );

    _Tick<false> * tickCache = NULL;

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

        std::tie(step.tickNext, step.initialized) = nextInitializedTickWithinOneWord(
            &(o->ticks),
            state.tick,
            o->tickSpacing,
            zeroForOne,
            newOperation,
            &tickCache
        );
        newOperation = false;
        // std::cerr << "Nxt tick of " << state.tick << " is " << step.tickNext->id << std::endl;
        // std::cerr << "Next " << zeroForOne << " of " << state.tick << " is " << step.tickNext->id << std::endl;
        // ensure that we do not overshoot the min/max tick, as the tick bitmap is not aware of these bounds
        if (step.tickNext->id < MIN_TICK) {
            step.tickNext = &MIN_TICK_OBJ_FALSE;
        } else if (step.tickNext->id > MAX_TICK) {
            step.tickNext = &MAX_TICK_OBJ_FALSE;
        }

        // get the price for the next tick
        step.sqrtPriceNextX96 = getSqrtRatioAtTick<uint160>(step.tickNext->id);
        // std::cerr << "Next tick id = " << step.tickNext->id << std::endl;
        // std::cerr << "nextPrice = " << step.sqrtPriceNextX96 << std::endl;


        // std::cerr << "Price " << sqrtPriceLimitX96.X96ToDouble() << " " << step.sqrtPriceNextX96.X96ToDouble() << std::endl;
        std::tie(state.sqrtPriceX96, step.amountIn, step.amountOut, step.feeAmount) = computeSwapStep(
            state.sqrtPriceX96,
            (zeroForOne ? step.sqrtPriceNextX96 < sqrtPriceLimitX96 : step.sqrtPriceNextX96 > sqrtPriceLimitX96)
                ? sqrtPriceLimitX96
                : step.sqrtPriceNextX96,
            state.liquidity,
            state.amountSpecifiedRemaining,
            o->fee
        );

        // std::cerr << "Price = " << state.sqrtPriceX96.X96ToDouble() << std::endl;

        if (exactInput) {
            state.amountSpecifiedRemaining -= int256((step.amountIn + step.feeAmount));
            state.amountCalculated = state.amountCalculated - int256(step.amountOut);
        } else {
            state.amountSpecifiedRemaining += int256(step.amountOut);
            state.amountCalculated = state.amountCalculated + int256(step.amountIn + step.feeAmount);
        }
        // std::cerr << "Remaining = " << state.amountSpecifiedRemaining << std::endl;
        // std::cerr << "Calculate = " << state.amountCalculated << std::endl;

        // shift tick if we reached the next price
        if (state.sqrtPriceX96 == step.sqrtPriceNextX96) {
            // std::cerr << "CASE 0" << std::endl;
            // if the tick is initialized, run the tick transition
            if (step.initialized) {
                int128 liquidityNet = step.tickNext->liquidityNet;
                // safe because liquidityNet cannot be type(int128).min
                if (zeroForOne) liquidityNet = -liquidityNet;
                state.liquidity = addDelta(state.liquidity, liquidityNet);
            }
            state.tick = zeroForOne ? step.tickNext->id - 1 : step.tickNext->id;
        } else if (state.sqrtPriceX96 != step.sqrtPriceStartX96) {
            // std::cerr << "CASE 1" << std::endl;
            // recompute unless we're on a lower tick boundary (i.e. already transitioned ticks), and haven't moved
            state.tick = getTickAtSqrtRatio(state.sqrtPriceX96);
            // std::cerr << "?? state.sqrtPriceX96 = " << state.sqrtPriceX96 << " tick = " << state.tick << std::endl;
        }
    }
    // std::cerr << "state.price = " << state.sqrtPriceX96.X96ToDouble() << std::endl;
    // std::cerr << "state.tick = " << state.tick << std::endl;
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
        // std::cerr << "final case 0" << std::endl;
        amount0 = amountSpecified - state.amountSpecifiedRemaining;
        amount1 = state.amountCalculated;
    } else {
        // std::cerr << "final case 1" << std::endl;
        amount0 = state.amountCalculated;
        amount1 = amountSpecified - state.amountSpecifiedRemaining;
    }
    // std::cerr << "==== End FUNCTION ==== \n\n" << std::endl;
    return std::make_pair(amount0, amount1);
}


std::pair<FloatType, FloatType> swap(
    Pool<true> *o,
    bool zeroForOne,
    FloatType & amountSpecified,
    FloatType & sqrtPriceLimitX96,
    bool effect)
{
    bool newOperation = true;

    if(fabs(amountSpecified) > EPS); else return std::make_pair(-1, -1);
    // require(fabs(amountSpecified) > EPS, "AS");

    __attribute__((aligned(64))) struct SwapRuntimeEnv{
        StepComputations<true> step;
        SwapState<true> state;
        FloatType liquidityCache;
        FloatType lastAmountSpecifiedRemaining;
        int cnt;
        FloatType liquidityNet;
        Slot0<true> slot0Start;
        SwapRuntimeEnv(){}
        ~SwapRuntimeEnv(){}
    } env;
    // prefetch_range(&env, sizeof(env));

    env.slot0Start = o->slot0;

    env.liquidityCache = o->liquidity;

    require(amountSpecified > 0 && !effect, "QA");

    env.state = SwapState<true>(
        amountSpecified,
        0,
        env.slot0Start.sqrtPriceX96,
        env.slot0Start.tick,
        env.liquidityCache
    );

    // continue swapping as long as we haven't used the entire input/output and haven't reached the price limit
    env.lastAmountSpecifiedRemaining = 0;
    env.cnt = 0;
    _Tick<true> * tickCache = NULL;

    while (fabs(env.state.amountSpecifiedRemaining) > EPS
        && env.state.amountSpecifiedRemaining > 0
        && fabs(env.state.sqrtPriceX96 - sqrtPriceLimitX96) > EPS) {
            // 一种更好的写法
            // (zeroForOne ? env.state.sqrtPriceX96 > sqrtPriceLimitX96 : env.state.sqrtPriceX96 < sqrtPriceLimitX96)
            // 未测试 固 未替换
            // 这个条件出问题再说吧

        if(fabs(env.lastAmountSpecifiedRemaining - env.state.amountSpecifiedRemaining) < EPS) env.cnt++;
        else env.cnt = 0;
        if (env.cnt > 2) break;

        // printf("%d Remainding = %.5lf\n", cnt, state.amountSpecifiedRemaining);

        env.lastAmountSpecifiedRemaining = env.state.amountSpecifiedRemaining;


        env.step.sqrtPriceStartX96 = env.state.sqrtPriceX96;

        // std::cerr << "? Query " << zeroForOne << " " << state.tick << std::endl;
        std::tie(env.step.tickNext, env.step.initialized) = nextInitializedTickWithinOneWord(
            &(o->ticks),
            env.state.tick,
            o->tickSpacing,
            zeroForOne,
            newOperation,
            &tickCache
        );
        newOperation = false;
        // std::cerr << zeroForOne << " Nxt tick of " << state.tick << " is " << step.tickNext->id << std::endl << std::endl;

        // ensure that we do not overshoot the min/max tick, as the tick bitmap is not aware of these bounds
        if (env.step.tickNext->id < MIN_TICK) {
            env.step.tickNext = &MIN_TICK_OBJ_TRUE;
        } else if (env.step.tickNext->id > MAX_TICK) {
            env.step.tickNext = &MAX_TICK_OBJ_TRUE;
        }

        // get the price for the next tick
        env.step.sqrtPriceNextX96 = getSqrtRatioAtTick<FloatType>(env.step.tickNext->id);

        // std::cerr << "Price " << sqrtPriceLimitX96 << " " << step.sqrtPriceNextX96 << std::endl;
        std::tie(env.state.sqrtPriceX96, env.step.amountIn, env.step.amountOut) = computeSwapStep(
            env.state.sqrtPriceX96,
            env.step.sqrtPriceNextX96,
            env.state.liquidity,
            env.state.amountSpecifiedRemaining,
            o->fee
        );

        // std::cerr << "Price = " << state.sqrtPriceX96 << std::endl;

        // if (env.exactInput) {
            env.state.amountSpecifiedRemaining -= env.step.amountIn;//  + env.step.feeAmount;
            env.state.amountCalculated = env.state.amountCalculated - env.step.amountOut;
        // } else {
        //     env.state.amountSpecifiedRemaining += env.step.amountOut;
        //     env.state.amountCalculated = env.state.amountCalculated + env.step.amountIn + env.step.feeAmount;
        // }
        // std::cerr << "Remaining = " << state.amountSpecifiedRemaining << std::endl;
        // std::cerr << "Calculate = " << state.amountCalculated << std::endl;
        // shift tick if we reached the next price
        if (fabs(env.state.sqrtPriceX96 - env.step.sqrtPriceNextX96) < EPS) {
            // std::cerr << "CASE 0" << std::endl;
            // if the tick is initialized, run the tick transition
            if (env.step.initialized) {
                env.liquidityNet = env.step.tickNext->liquidityNet;
                // if we're moving leftward, we interpret liquidityNet as the opposite sign
                // safe because liquidityNet cannot be type(int128).min
                if (zeroForOne) env.liquidityNet = -env.liquidityNet;

                env.state.liquidity += env.liquidityNet;
            }

            env.state.tick = zeroForOne ? env.step.tickNext->id - 1 : env.step.tickNext->id;
        } else if (fabs( env.state.sqrtPriceX96 - env.step.sqrtPriceStartX96) > EPS) {
            // std::cerr << "CASE 1" << std::endl;
            // recompute unless we're on a lower tick boundary (i.e. already transitioned ticks), and haven't moved

            if(zeroForOne) {
                while(getSqrtRatioAtTick<FloatType>(env.state.tick - 1) >= env.state.sqrtPriceX96) {
                    env.state.tick--;
                }
            } else {
                while(getSqrtRatioAtTick<FloatType>(env.state.tick + 1) <= env.state.sqrtPriceX96) {
                    env.state.tick++;
                }
            }

            // env.state.tick = getTickAtSqrtRatio(env.state.sqrtPriceX96);

            // std::cerr << "?? state.sqrtPriceX96 = " << state.sqrtPriceX96 << " tick = " << state.tick << std::endl;
        }
    }

    FloatType amount0, amount1;
    if (zeroForOne) {
        // std::cerr << "final case 0" << std::endl;
        amount0 = amountSpecified - env.state.amountSpecifiedRemaining;
        amount1 = env.state.amountCalculated;
    } else {
        // std::cerr << "final case 1" << std::endl;
        amount0 = env.state.amountCalculated;
        amount1 = amountSpecified - env.state.amountSpecifiedRemaining;
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
    int24 tickLowerId,
    int24 tickUpperId,
    typename std::conditional<enable_float, FloatType, int128>::type liquidityDelta
) {
#ifdef DEBUG
    std::cerr << "_updatePosition(tickLower = " << tickLowerId << ", tickUpper = " << tickUpperId << ", liquidityDelta = " << liquidityDelta << ");" << std::endl;
#endif
    // if we need to update the ticks, do it
    bool erasedLower;
    bool erasedUpper;
    // std::cerr<< "============== One Round Start ============== " << std::endl;
    if (!isZero(liquidityDelta)) {
        auto tickLower = fetchTick(&o->ticks, tickLowerId);
        auto tickUpper = fetchTick(&o->ticks, tickUpperId);

        erasedLower = updateTick(
            tickLower,
            liquidityDelta,
            false,
            o->maxLiquidityPerTick
        );
        erasedUpper = updateTick(
            tickUpper,
            liquidityDelta,
            true,
            o->maxLiquidityPerTick
        );
        if (erasedLower) {
            // std::cerr << "!!erase " << tickLowerId << std::endl;
            eraseTick(&o->ticks, tickLowerId, o->tickSpacing);
        }
        if (erasedUpper) {
            // std::cerr << "!!erase " << tickUpperId << std::endl;
            eraseTick(&o->ticks, tickUpperId, o->tickSpacing);
        }
    }
    // std::cerr << "mid check" << std::endl;
    // clear any tick data that is no longer needed
    if (liquidityDelta < 0) {
        if (erasedLower) {
            clearTick(&o->ticks, tickLowerId);
        }
        if (erasedUpper) {
            clearTick(&o->ticks, tickUpperId);
        }
    }
    // std::cerr<< "============== One Round END  ============== " << std::endl;
}

template<bool enable_float>
typename std::conditional<enable_float, std::tuple<FloatType, FloatType>, std::tuple<int256, int256>>::type  \
_modifyPosition(Pool<enable_float> * o, ModifyPositionParams<enable_float> params) {
    checkTicks(params.tickLower, params.tickUpper);
#ifdef DEBUG
    std::cerr << "_modifyPosition("; params.print(); std::cerr << ")" << std::endl;
#endif
    Slot0<enable_float> &_slot0 = o->slot0; // SLOAD for gas optimization

    _updatePosition<enable_float>(
        o,
        params.tickLower,
        params.tickUpper,
        params.liquidityDelta
    );

    typename std::conditional<enable_float, FloatType, int256>::type amount0 = 0, amount1 = 0;

    // std::cerr << "?? " << params.tickLower << " " << params.tickUpper << " " << params.liquidityDelta << std::endl;

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
            // std::cerr << "Price = " <<  _slot0.sqrtPriceX96 << std::endl;
            // std::cerr << "value0 = " << params.tickLower << " " << getSqrtRatioAtTick<PriceType>(params.tickLower) << std::endl;
            // std::cerr << "value1 = " << params.tickUpper << " " << getSqrtRatioAtTick<PriceType>(params.tickUpper) << std::endl;
            // std::cerr << "liqdata = " << params.liquidityDelta << std::endl;
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
            // std::cerr << "amount0 = " << amount0 << std::endl;
            // std::cerr << "amount1 = " << amount1 << std::endl;
            o->liquidity = addDelta(liquidityBefore, params.liquidityDelta);
        } else {
            // current tick is above the passed range; liquidity can only become in range by crossing from right to
            // left, when we'll need _more_ token1 (it's becoming more valuable) so user must provide it
            // std::cerr << "case last" << std::endl;
            // std::cerr << "value = " << getSqrtRatioAtTick<PriceType>(params.tickLower) << " " << getSqrtRatioAtTick<PriceType>(params.tickUpper) << std::endl;
            // std::cerr << "liqdata = " << params.liquidityDelta << std::endl;
            amount1 = getAmount1Delta(
                getSqrtRatioAtTick<PriceType>(params.tickLower),
                getSqrtRatioAtTick<PriceType>(params.tickUpper),
                params.liquidityDelta
            );
            // std::cerr << "amount1 = " << amount1 << std::endl;
        }
    }
    return std::make_tuple(amount0, amount1);
}



template<bool enable_float>
typename std::conditional<enable_float, std::pair<FloatType, FloatType>, std::pair<uint256, uint256>>::type mint(
    Pool<enable_float> * o,
    int24 tickLower,
    int24 tickUpper,
    typename std::conditional<enable_float, FloatType, uint128>::type amount
) {
    typedef typename std::conditional<enable_float, FloatType, int256>::type AmountType;
    typedef typename std::conditional<enable_float, FloatType, uint256>::type FinalAmountType;

    require(amount > 0, "AMZ0");
    AmountType amount0Int, amount1Int;
    std::tie(amount0Int, amount1Int) = _modifyPosition<enable_float>(
        o,
        ModifyPositionParams<enable_float>(tickLower, tickUpper, (AmountType)(amount))
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
#ifdef DEBUG
    // [ShuYuMo]: 求求来个大佬 写个什么牛逼的宏 把下面的这一坨简化了吧。
    std::cerr << "burn(tickLower = " << tickLower << ", " << "tickUpper = "  << tickUpper << ", amount = " << amount << ")" << std::endl;
#endif
    typedef typename std::conditional<enable_float, FloatType, int256>::type AmountType;
    typedef typename std::conditional<enable_float, FloatType, uint256>::type FinalAmountType;

    AmountType amount0Int, amount1Int;
    std::tie(amount0Int, amount1Int) = _modifyPosition(
        o,
        ModifyPositionParams<enable_float>(tickLower, tickUpper, -(AmountType)(amount))
    );

    FinalAmountType amount0 = (FinalAmountType)(-amount0Int);
    FinalAmountType amount1 = (FinalAmountType)(-amount1Int);
    return std::make_pair(amount0, amount1);
}



#endif