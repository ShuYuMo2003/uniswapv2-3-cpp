#ifndef headerfilepool
#define headerfilepool

#include <fstream>
#include <cmath>
#include <cstdlib> // For debug pause only.

#include "consts.h"
#include "global.h"
#include "types.h"
#include "util.h"

#include "oracle.h"
#include "position.h"
#include "swapmath.h"
#include "tick.h"
#include "tick_bitmap.h"
#include "tickmath.h"
#include "transfer_helper.h"
#include "liquiditymath.h"

class Pool {
public:
    /// @inheritdoc IUniswapV3PoolImmutables
    // const address factory;
    /// @inheritdoc IUniswapV3PoolImmutables
    // const address token0;
    /// @inheritdoc IUniswapV3PoolImmutables
    // const address token1;
    /// @inheritdoc IUniswapV3PoolImmutables
    uint24 fee;
    /// @inheritdoc IUniswapV3PoolImmutables
    int24 tickSpacing;
    /// @inheritdoc IUniswapV3PoolImmutables
    uint128 maxLiquidityPerTick;
    /// @inheritdoc IUniswapV3PoolState
    Slot0 slot0;
    /// @inheritdoc IUniswapV3PoolState
    uint128 liquidity;
    /// @inheritdoc IUniswapV3PoolState
    // uint256 feeGrowthGlobal0X128;
    /// @inheritdoc IUniswapV3PoolState
    // uint256 feeGrowthGlobal1X128;
    /// @inheritdoc IUniswapV3PoolState
    // Observations observations;
    /// @inheritdoc IUniswapV3PoolState
    Ticks ticks;
    /// @inheritdoc IUniswapV3PoolState

    // If some error occurs because of bitmap, just replace the codes below to `TickBitmap tickBitmap;`
    TickBitMapBaseOnVector tickBitmap;

    /// @inheritdoc IUniswapV3PoolState
    // Positions positions;
    // accumulated protocol fees in token0/token1 units
    // struct ProtocolFees {
    //     uint128 token0;
    //     uint128 token1;
    // };
    /// @inheritdoc IUniswapV3PoolState
    // ProtocolFees protocolFees;
    uint256 balance0, balance1;
    // Pool(address factory, address token0, address token1, uint24 fee, int24 tickSpacing, uint128 maxLiquidityPerTick)
    //     : factory(factory), token0(token0), token1(token1), fee(fee), tickSpacing(tickSpacing), maxLiquidityPerTick(maxLiquidityPerTick) {
    //     // feeGrowthGlobal0X128 = feeGrowthGlobal1X128 =
    //     liquidity = 0;
    // }
    Pool() {}
    Pool(uint24 fee, int24 tickSpacing, uint128 maxLiquidityPerTick)
        : fee(fee), tickSpacing(tickSpacing), maxLiquidityPerTick(maxLiquidityPerTick) {
        // feeGrowthGlobal0X128 = feeGrowthGlobal1X128 =
        liquidity = 0;
    }
    friend std::istream& operator>>(std::istream& is, Pool& pool) {
        is >> pool.fee >> pool.tickSpacing >> pool.maxLiquidityPerTick
            >> pool.liquidity >> pool.slot0 >> pool.ticks >> pool.tickBitmap;
        return is;
    }
    friend std::ostream& operator<<(std::ostream& os, const Pool& pool) {
        os << pool.fee << " " << pool.tickSpacing << " " << pool.maxLiquidityPerTick << " " << pool.liquidity << std::endl;
        os << pool.slot0 << std::endl;
        os << pool.ticks << pool.tickBitmap;
        return os;
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
        // , positions = o.positions;
    }
    const Pool & operator=(const Pool &o) {
        copyFrom(o);
        return *this;
    }
    // Pool(const Pool &o) : factory(o.factory), token0(o.token0), token1(o.token1), fee(o.fee), tickSpacing(o.tickSpacing), maxLiquidityPerTick(o.maxLiquidityPerTick) {
    Pool(const Pool &o) : fee(o.fee), tickSpacing(o.tickSpacing), maxLiquidityPerTick(o.maxLiquidityPerTick) {
        copyFrom(o);
    }
    /// @dev Returns the block timestamp truncated to 32 bits, i.e. mod 2**32. This method is overridden in tests.
    uint32 _blockTimestamp() {
        return uint32(block.timestamp); // truncation is desired
    }
    /// @inheritdoc IUniswapV3PoolActions
    /// @dev not locked because it initializes unlocked
    int24 initialize(uint160 sqrtPriceX96) {
        require(slot0.sqrtPriceX96 == 0, "AI");

        int24 tick = getTickAtSqrtRatio(sqrtPriceX96);

        // uint16 cardinality, cardinalityNext;
        // std::tie(cardinality, cardinalityNext) = observations.initialize(_blockTimestamp());

        // slot0 = Slot0(sqrtPriceX96, tick, 0, cardinality, cardinalityNext, 0, true);
        // slot0 = Slot0(sqrtPriceX96, tick, 0);
        slot0 = Slot0(sqrtPriceX96, tick);

        return tick;
        // emit Initialize(sqrtPriceX96, tick);
    }
    std::pair<int256, int256> swap(
        address recipient,
        bool zeroForOne,
        int256 amountSpecified,
        uint160 sqrtPriceLimitX96,
        bytes32 data,
        bool effect = true)
    {
        // std::cerr << "===================================== new swap ==========================" << std::endl;
        require(amountSpecified != 0, "AS");

        Slot0 slot0Start = slot0;

        // require(slot0Start.unlocked, "LOK");
        // std::cout << zeroForOne << " " << sqrtPriceLimitX96 << " " << slot0Start.sqrtPriceX96 << " " << MIN_SQRT_RATIO << " " << MAX_SQRT_RATIO << std::endl;
        require(
            zeroForOne
                ? sqrtPriceLimitX96 < slot0Start.sqrtPriceX96 && sqrtPriceLimitX96 > MIN_SQRT_RATIO
                : sqrtPriceLimitX96 > slot0Start.sqrtPriceX96 && sqrtPriceLimitX96 < MAX_SQRT_RATIO,
            "SPL"
        );

        // slot0.unlocked = false;

        SwapCache cache = SwapCache(
            // zeroForOne ? (slot0Start.feeProtocol % 16) : (slot0Start.feeProtocol >> 4),
            liquidity
            // block.timestamp, // _blockTimestamp(),
            // 0,
            // 0,
            // false
        );

        bool exactInput = amountSpecified > 0;

        SwapState state = SwapState(
            amountSpecified,
            0,
            slot0Start.sqrtPriceX96,
            slot0Start.tick,
            // zeroForOne ? feeGrowthGlobal0X128 : feeGrowthGlobal1X128,
            // 0,
            cache.liquidityStart
        );

        // continue swapping as long as we haven't used the entire input/output and haven't reached the price limit
        while (state.amountSpecifiedRemaining != 0 && state.sqrtPriceX96 != sqrtPriceLimitX96) {
            // std::cerr << "Remaining: " << state.amountSpecifiedRemaining << std::endl;
            StepComputations step;

            step.sqrtPriceStartX96 = state.sqrtPriceX96;

            // std::cout << "------- nextInitializedTickWithinOneWord params -------\n"
            //     << state.tick << " " << tickSpacing << " " << zeroForOne
            //     << "\n------- nextInitializedTickWithinOneWord params end -------\n";
            std::tie(step.tickNext, step.initialized) = tickBitmap.nextInitializedTickWithinOneWord(
                state.tick,
                tickSpacing,
                zeroForOne
            );
            // std::cout << step.tickNext << " " << step.initialized << std::endl;
            // assert(step.initialized == true);

            // ensure that we do not overshoot the min/max tick, as the tick bitmap is not aware of these bounds
            if (step.tickNext < MIN_TICK) {
                step.tickNext = MIN_TICK;
            } else if (step.tickNext > MAX_TICK) {
                step.tickNext = MAX_TICK;
            }

            // get the price for the next tick
            step.sqrtPriceNextX96 = getSqrtRatioAtTick(step.tickNext);

            // std::cout << step.tickNext << " " << step.sqrtPriceNextX96 << std::endl;

            // compute values to swap to the target tick, price limit, or point where input/output amount is exhausted

            // std::cout << "==== " << state.sqrtPriceX96 << " " << ((zeroForOne ? step.sqrtPriceNextX96 < sqrtPriceLimitX96 : step.sqrtPriceNextX96 > sqrtPriceLimitX96)
            //         ? sqrtPriceLimitX96
            //         : step.sqrtPriceNextX96) << " " << state.liquidity << " " << state.amountSpecifiedRemaining << " " << fee << std::endl;
            // if (state.liquidity == "353994491063406687") state.liquidity = "353265040822481228";
            std::tie(state.sqrtPriceX96, step.amountIn, step.amountOut, step.feeAmount) = computeSwapStep(
                state.sqrtPriceX96,
                (zeroForOne ? step.sqrtPriceNextX96 < sqrtPriceLimitX96 : step.sqrtPriceNextX96 > sqrtPriceLimitX96)
                    ? sqrtPriceLimitX96
                    : step.sqrtPriceNextX96,
                state.liquidity,
                state.amountSpecifiedRemaining,
                fee
            );

            // std::cout << "state.sqrtPriceX96: " <<  (state.sqrtPriceX96.X96ToDouble()) << std::endl;
            // std::cout << "step.amountIn: " << (step.amountIn)  << std::endl;
            // std::cout << "step.amountOut: " << step.amountOut << std::endl;
            // std::cout << "step.feeAmount: " << step.feeAmount << std::endl;
            // std::cout << "---- " << state.sqrtPriceX96 << " " << step.amountIn << " " << step.amountOut << " " << step.feeAmount << std::endl;

            if (exactInput) {
                state.amountSpecifiedRemaining -= int256((step.amountIn + step.feeAmount));
                state.amountCalculated = state.amountCalculated - int256(step.amountOut);
            } else {
                state.amountSpecifiedRemaining += int256(step.amountOut);
                state.amountCalculated = state.amountCalculated + int256(step.amountIn + step.feeAmount);
            }

            // std::cout << state.amountSpecifiedRemaining << " " << state.amountCalculated << std::endl;

            // if the protocol fee is on, calculate how much is owed, decrement feeAmount, and increment protocolFee
            // if (cache.feeProtocol > 0) {
            //     uint256 delta = step.feeAmount / cache.feeProtocol;
            //     step.feeAmount -= delta;
            //     state.protocolFee += uint128(delta);
            // }

            // update global fee tracker
            // if (state.liquidity > 0)
            //     state.feeGrowthGlobalX128 += mulDiv(step.feeAmount, Q128, state.liquidity);

            // shift tick if we reached the next price
            if (state.sqrtPriceX96 == step.sqrtPriceNextX96) {
                // if the tick is initialized, run the tick transition
                if (step.initialized) {
                    // check for the placeholder value, which we replace with the actual value the first time the swap
                    // crosses an initialized tick
                    // if (!cache.computedLatestObservation) {
                    //     std::tie(cache.tickCumulative, cache.secondsPerLiquidityCumulativeX128) = observations.observeSingle(
                    //         cache.blockTimestamp,
                    //         0,
                    //         slot0Start.tick,
                    //         slot0Start.observationIndex,
                    //         cache.liquidityStart,
                    //         slot0Start.observationCardinality
                    //     );
                    //     cache.computedLatestObservation = true;
                    // }
                    int128 liquidityNet = ticks.cross(
                        step.tickNext
                        // (zeroForOne ? state.feeGrowthGlobalX128 : feeGrowthGlobal0X128),
                        // (zeroForOne ? feeGrowthGlobal1X128 : state.feeGrowthGlobalX128),
                        // cache.secondsPerLiquidityCumulativeX128,
                        // cache.tickCumulative,
                        // cache.blockTimestamp
                    );
                    // std::cout << state.tick << " " << step.tickNext << " " << liquidityNet << std::endl;
                    // if we're moving leftward, we interpret liquidityNet as the opposite sign
                    // safe because liquidityNet cannot be type(int128).min
                    if (zeroForOne) liquidityNet = -liquidityNet;

                    // std::cout << state.liquidity << " ???? " << liquidityNet << std::endl;
                    // std::cout << state.liquidity + liquidityNet << std::endl;

                    state.liquidity = addDelta(state.liquidity, liquidityNet);
                }

                state.tick = zeroForOne ? step.tickNext - 1 : step.tickNext;
            } else if (state.sqrtPriceX96 != step.sqrtPriceStartX96) {
                // recompute unless we're on a lower tick boundary (i.e. already transitioned ticks), and haven't moved
                state.tick = getTickAtSqrtRatio(state.sqrtPriceX96);
            }

            // std::cerr << "END Remaining: " << state.amountSpecifiedRemaining << std::endl;
        }

        // std::cout << "---- " << state.tick << " " << slot0Start.tick << std::endl;

        // update tick and write an oracle entry if the tick change
        if(effect) {
            if (state.tick != slot0Start.tick) {
                // uint16 observationIndex, observationCardinality;
                // std::tie(observationIndex, observationCardinality) = observations.write(
                //     slot0Start.observationIndex,
                //     cache.blockTimestamp,
                //     slot0Start.tick,
                //     cache.liquidityStart,
                //     slot0Start.observationCardinality,
                //     slot0Start.observationCardinalityNext
                // );
                slot0.sqrtPriceX96 = state.sqrtPriceX96;
                slot0.tick = state.tick;
                // slot0.observationIndex = observationIndex;
                // slot0.observationCardinality = observationCardinality;
            } else {
                // otherwise just update the price
                slot0.sqrtPriceX96 = state.sqrtPriceX96;
            }

            // std::cout << "---- " << state.tick << " " << slot0.tick << std::endl;

            // update liquidity if it changed
            if (cache.liquidityStart != state.liquidity) liquidity = state.liquidity;
        }


        // update fee growth global and, if necessary, protocol fees
        // overflow is acceptable, protocol has to withdraw before it hits type(uint128).max fees
        // if (zeroForOne) {
        //     feeGrowthGlobal0X128 = state.feeGrowthGlobalX128;
        //     if (state.protocolFee > 0) protocolFees.token0 += state.protocolFee;
        // } else {
        //     feeGrowthGlobal1X128 = state.feeGrowthGlobalX128;
        //     if (state.protocolFee > 0) protocolFees.token1 += state.protocolFee;
        // }

        int256 amount0, amount1;
        if (zeroForOne == exactInput) {
            amount0 = amountSpecified - state.amountSpecifiedRemaining;
            amount1 = state.amountCalculated;
        } else {
            amount0 = state.amountCalculated;
            amount1 = amountSpecified - state.amountSpecifiedRemaining;
        }
        // do the transfers and collect payment
        // if (zeroForOne) {
        //     if (amount1 < 0) safeTransfer(token1, recipient, uint256(-amount1));

        //     uint256 balance0Before = balance0;
        //     IUniswapV3SwapCallback(msg.sender).uniswapV3SwapCallback(amount0, amount1, data);
        //     require(balance0Before.add(uint256(amount0)) <= balance0(), 'IIA');
        // } else {
        //     if (amount0 < 0) safeTransfer(token0, recipient, uint256(-amount0));

        //     uint256 balance1Before = balance1;
        //     IUniswapV3SwapCallback(msg.sender).uniswapV3SwapCallback(amount0, amount1, data);
        //     require(balance1Before.add(uint256(amount1)) <= balance1(), 'IIA');
        // }

        // emit Swap(msg.sender, recipient, amount0, amount1, state.sqrtPriceX96, state.liquidity, state.tick);
        // slot0.unlocked = true;
        return std::make_pair(amount0, amount1);
    }

    std::pair<double, double> swap_effectless(
        address recipient,
        bool zeroForOne,
        int256 raw_amountSpecified,
        uint160 raw_sqrtPriceLimitX96,
        bytes32 data)
    {
        double sqrtPriceLimitX96 = raw_sqrtPriceLimitX96.X96ToDouble();
        double amountSpecified   = raw_amountSpecified.ToDouble();

        require(fabs(amountSpecified) > EPS, "AS");

        Slot0_float slot0Start(slot0.sqrtPriceX96, slot0.tick);

        require(
            zeroForOne
                ? raw_sqrtPriceLimitX96 < slot0.sqrtPriceX96 && raw_sqrtPriceLimitX96 > MIN_SQRT_RATIO
                : raw_sqrtPriceLimitX96 > slot0.sqrtPriceX96 && raw_sqrtPriceLimitX96 < MAX_SQRT_RATIO,
            "SPL"
        );
/*
        std::cerr << sqrtPriceLimitX96 << std::endl;

        if(
            zeroForOne
                ? sqrtPriceLimitX96 < slot0Start.sqrtPriceX96 && sqrtPriceLimitX96 > MIN_SQRT_RATIO_FLOAT
                : sqrtPriceLimitX96 > slot0Start.sqrtPriceX96 && sqrtPriceLimitX96 < MAX_SQRT_RATIO_FLOAT
        ); else {
            std::cout << "================================ FAIL ================================" << std::endl;
            std::cout << zeroForOne << std::endl;
            printf("Price = %.50lf\n", sqrtPriceLimitX96);
            std::cout << sqrtPriceLimitX96 << " " << slot0Start.sqrtPriceX96 << " " << sqrtPriceLimitX96 << " " <<  MAX_SQRT_RATIO_FLOAT << std::endl;
            assert(false);
        }

        // std::cerr <<
*/
        // printf("START_PRICE = %.30lf\n", slot0Start.sqrtPriceX96);

        SwapCache_float cache = SwapCache_float(liquidity);

        bool exactInput = amountSpecified > 0;


        SwapState_float state = SwapState_float(
            amountSpecified,
            0,
            slot0Start.sqrtPriceX96,
            slot0Start.tick,
            cache.liquidityStart
        );

        // printf("liq:\n\t%.2lf\n\t%.2lf\n", cache.liquidityStart, state.liquidity);

        // continue swapping as long as we haven't used the entire input/output and haven't reached the price limit
        double lastAmountSpecifiedRemaining = 0;

        while ((fabs(lastAmountSpecifiedRemaining - state.amountSpecifiedRemaining) > EPS)
            && fabs(state.amountSpecifiedRemaining) > EPS
            && fabs(state.sqrtPriceX96 - sqrtPriceLimitX96) > EPS) {

            lastAmountSpecifiedRemaining = state.amountSpecifiedRemaining;

            // printf("Remaining: %.20lf\n", state.amountSpecifiedRemaining);
            StepComputations_float step;

            step.sqrtPriceStartX96 = state.sqrtPriceX96;

            std::tie(step.tickNext, step.initialized) = tickBitmap.nextInitializedTickWithinOneWord(
                state.tick,
                tickSpacing,
                zeroForOne
            );

            // ensure that we do not overshoot the min/max tick, as the tick bitmap is not aware of these bounds
            if (step.tickNext < MIN_TICK) {
                step.tickNext = MIN_TICK;
            } else if (step.tickNext > MAX_TICK) {
                step.tickNext = MAX_TICK;
            }

            // get the price for the next tick
            step.sqrtPriceNextX96 = getSqrtRatioAtTick(step.tickNext).X96ToDouble();

            // std::cout << "call" << std::endl;
            std::tie(state.sqrtPriceX96, step.amountIn, step.amountOut, step.feeAmount) = computeSwapStep_float(
                state.sqrtPriceX96,
                (zeroForOne ? step.sqrtPriceNextX96 < sqrtPriceLimitX96 : step.sqrtPriceNextX96 > sqrtPriceLimitX96)
                    ? sqrtPriceLimitX96
                    : step.sqrtPriceNextX96,
                state.liquidity,
                state.amountSpecifiedRemaining,
                fee
            );

            // printf("state.sqrtPriceX96: %.2lf\n", state.sqrtPriceX96);
            // printf("step.amountIn: %.2lf\n", step.amountIn);
            // printf("step.amountOut: %.2lf\n", step.amountOut);
            // printf("step.feeAmount: %.2lf\n", step.feeAmount);


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
                    double liquidityNet = ticks.cross(step.tickNext).ToDouble();

                    // if we're moving leftward, we interpret liquidityNet as the opposite sign
                    // safe because liquidityNet cannot be type(int128).min
                    if (zeroForOne) liquidityNet = -liquidityNet;

                    state.liquidity = addDelta_float(state.liquidity, liquidityNet);
                }

                state.tick = zeroForOne ? step.tickNext - 1 : step.tickNext;
            } else if (fabs( state.sqrtPriceX96 - step.sqrtPriceStartX96) > EPS) {
                // recompute unless we're on a lower tick boundary (i.e. already transitioned ticks), and haven't moved
                state.tick = getTickAtSqrtRatio_float(state.sqrtPriceX96);
            }

            // printf("END Remaining: %.50lf\n", state.amountSpecifiedRemaining);


        }
/*
        // !! mustn't have any effects on the ticks.
        // !! This function is only able to get `fucking shit grabage result`.

        // update tick and write an oracle entry if the tick change
        if(effect) {
            if (state.tick != slot0Start.tick) {
                slot0.sqrtPriceX96 = state.sqrtPriceX96;
                slot0.tick = state.tick;
            } else {
                // otherwise just update the price
                slot0.sqrtPriceX96 = state.sqrtPriceX96;
            }

            // update liquidity if it changed
            if (cache.liquidityStart != state.liquidity) liquidity = state.liquidity;
        }
*/
        double amount0, amount1;
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

    /// @dev Effect some changes to a position
    /// @param params the position details and the change to the position's liquidity to effect
    /// @return position a storage pointer referencing the position with the given owner and tick range
    /// @return amount0 the amount of token0 owed to the pool, negative if the pool should pay the recipient
    /// @return amount1 the amount of token1 owed to the pool, negative if the pool should pay the recipient
    // std::tuple<Position, int256, int256> _modifyPosition(ModifyPositionParams params) {
    std::tuple<int256, int256> _modifyPosition(ModifyPositionParams params) {
        checkTicks(params.tickLower, params.tickUpper);
        // params.print();

        Slot0 &_slot0 = slot0; // SLOAD for gas optimization

        // Position &position =
        _updatePosition(
            params.owner,
            params.tickLower,
            params.tickUpper,
            params.liquidityDelta,
            _slot0.tick
        );

        // position.print();

        int256 amount0 = 0, amount1 = 0;

        if (params.liquidityDelta != 0) {
            if (_slot0.tick < params.tickLower) {
                // current tick is below the passed range; liquidity can only become in range by crossing from left to
                // right, when we'll need _more_ token0 (it's becoming more valuable) so user must provide it
                amount0 = getAmount0Delta(
                    getSqrtRatioAtTick(params.tickLower),
                    getSqrtRatioAtTick(params.tickUpper),
                    params.liquidityDelta
                );
            } else if (_slot0.tick < params.tickUpper) {
                // current tick is inside the passed range
                uint128 liquidityBefore = liquidity; // SLOAD for gas optimization

                // write an oracle entry
                // std::tie(slot0.observationIndex, slot0.observationCardinality) = observations.write(
                //     _slot0.observationIndex,
                //     _blockTimestamp(),
                //     _slot0.tick,
                //     liquidityBefore,
                //     _slot0.observationCardinality,
                //     _slot0.observationCardinalityNext
                // );

                // std::cout << _slot0.sqrtPriceX96 << " " << getSqrtRatioAtTick(params.tickUpper) << " " << params.liquidityDelta << std::endl;
                amount0 = getAmount0Delta(
                    _slot0.sqrtPriceX96,
                    getSqrtRatioAtTick(params.tickUpper),
                    params.liquidityDelta
                );
                // std::cout << getSqrtRatioAtTick(params.tickLower) << " " << _slot0.sqrtPriceX96 << " " << params.liquidityDelta << std::endl;
                amount1 = getAmount1Delta(
                    getSqrtRatioAtTick(params.tickLower),
                    _slot0.sqrtPriceX96,
                    params.liquidityDelta
                );

                liquidity = addDelta(liquidityBefore, params.liquidityDelta);
            } else {
                // current tick is above the passed range; liquidity can only become in range by crossing from right to
                // left, when we'll need _more_ token1 (it's becoming more valuable) so user must provide it
                amount1 = getAmount1Delta(
                    getSqrtRatioAtTick(params.tickLower),
                    getSqrtRatioAtTick(params.tickUpper),
                    params.liquidityDelta
                );
            }
        }
        return std::make_tuple(amount0, amount1);
        // return std::make_tuple(position, amount0, amount1);
    }

    /// @dev Gets and updates a position with the given liquidity delta
    /// @param owner the owner of the position
    /// @param tickLower the lower tick of the position's tick range
    /// @param tickUpper the upper tick of the position's tick range
    /// @param tick the current tick, passed to avoid sloads
    // Position& _updatePosition(
    void _updatePosition(
        address owner,
        int24 tickLower,
        int24 tickUpper,
        int128 liquidityDelta,
        int24 tick
    ) {
        // std::cout << owner << " " << tickLower << " " << tickUpper << " " << liquidityDelta << " " << tick << std::endl;

        // Position &position = positions.get(owner, tickLower, tickUpper);

        // uint256 _feeGrowthGlobal0X128 = feeGrowthGlobal0X128; // SLOAD for gas optimization
        // uint256 _feeGrowthGlobal1X128 = feeGrowthGlobal1X128; // SLOAD for gas optimization

        // if we need to update the ticks, do it
        bool flippedLower;
        bool flippedUpper;
        if (liquidityDelta != 0) {
            uint32 time = _blockTimestamp();
            // int56 tickCumulative;
            // uint160 secondsPerLiquidityCumulativeX128;
            // std::tie(tickCumulative, secondsPerLiquidityCumulativeX128) = observations.observeSingle(
            //     time,
            //     0,
            //     slot0.tick,
            //     slot0.observationIndex,
            //     liquidity,
            //     slot0.observationCardinality
            // );

            flippedLower = ticks.update(
                tickLower,
                tick,
                liquidityDelta,
                // _feeGrowthGlobal0X128,
                // _feeGrowthGlobal1X128,
                // secondsPerLiquidityCumulativeX128,
                // tickCumulative,
                time,
                false,
                maxLiquidityPerTick
            );
            flippedUpper = ticks.update(
                tickUpper,
                tick,
                liquidityDelta,
                // _feeGrowthGlobal0X128,
                // _feeGrowthGlobal1X128,
                // secondsPerLiquidityCumulativeX128,
                // tickCumulative,
                time,
                true,
                maxLiquidityPerTick
            );

            // std::cout << flippedLower << " " << flippedUpper << std::endl;

            if (flippedLower) {
                tickBitmap.flipTick(tickLower, tickSpacing);
            }
            if (flippedUpper) {
                tickBitmap.flipTick(tickUpper, tickSpacing);
            }
        }

        // puts("???");

        // uint256 feeGrowthInside0X128, feeGrowthInside1X128;
        // std::tie(feeGrowthInside0X128, feeGrowthInside1X128) = ticks.getFeeGrowthInside(
        //     tickLower,
        //     tickUpper,
        //     tick,
        //     _feeGrowthGlobal0X128,
        //     _feeGrowthGlobal1X128
        // );

        // position.update(liquidityDelta, feeGrowthInside0X128, feeGrowthInside1X128);

        // clear any tick data that is no longer needed
        if (liquidityDelta < 0) {
            if (flippedLower) {
                ticks.clear(tickLower);
            }
            if (flippedUpper) {
                ticks.clear(tickUpper);
            }
        }

        // return position;
    }

    /// @inheritdoc IUniswapV3PoolActions
    /// @dev noDelegateCall is applied indirectly via _modifyPosition
    std::pair<uint256, uint256> mint(
        address recipient,
        int24 tickLower,
        int24 tickUpper,
        uint128 amount,
        bytes32 data
    ) {
        require(amount > 0, "AMZ0");
        int256 amount0Int, amount1Int;
        // std::tie(std::ignore, amount0Int, amount1Int) = _modifyPosition(
        std::tie(amount0Int, amount1Int) = _modifyPosition(
            ModifyPositionParams(recipient, tickLower, tickUpper, int256(amount))
        );

        // int24 tickNext;
        // bool initialized;
        // std::tie(tickNext, initialized) = tickBitmap.nextInitializedTickWithinOneWord(
        //     slot0.tick,
        //     tickSpacing,
        //     0
        // );
        // uint160 sqrtPriceNextX96 = getSqrtRatioAtTick(tickNext);
        // std::cout << tickNext << " " << initialized << " " << sqrtPriceNextX96 << std::endl;

        uint256 amount0 = uint256(amount0Int), amount1 = uint256(amount1Int);

        // uint256 balance0Before;
        // uint256 balance1Before;
        // if (amount0 > 0) balance0Before = balance0;
        // if (amount1 > 0) balance1Before = balance1;
        // IUniswapV3MintCallback(msg.sender).uniswapV3MintCallback(amount0, amount1, data);
        // if (amount0 > 0) require(balance0Before + amount0 <= balance0, "M0");
        // if (amount1 > 0) require(balance1Before + amount1 <= balance1, "M1");

        return std::make_pair(amount0, amount1);
        // std::cout << amount0 << " " << amount1 << std::endl;
        // emit Mint(msg.sender, recipient, tickLower, tickUpper, amount, amount0, amount1);
    }

    /// @inheritdoc IUniswapV3PoolActions
    /// @dev noDelegateCall is applied indirectly via _modifyPosition
    std::pair<uint256, uint256> burn(
        int24 tickLower,
        int24 tickUpper,
        uint128 amount
    ) {
        int256 amount0Int, amount1Int;
        std::tie(amount0Int, amount1Int) = _modifyPosition(
            ModifyPositionParams(msg.sender, tickLower, tickUpper, -int256(amount))
        );
        // puts("???");

        uint256 amount0 = uint256(-amount0Int);
        uint256 amount1 = uint256(-amount1Int);

        // if (amount0 > 0 || amount1 > 0) {
        //     Position &p = positions.get(msg.sender, tickLower, tickUpper);
        //     p.tokensOwed0 += uint128(amount0);
        //     p.tokensOwed1 += uint128(amount1);
        // }
        return std::make_pair(amount0, amount1);
        // emit Burn(msg.sender, tickLower, tickUpper, amount, amount0, amount1);
    }

    /// @inheritdoc IUniswapV3PoolActions
    // std::pair<uint128, uint128> collect(
    //     address recipient,
    //     int24 tickLower,
    //     int24 tickUpper,
    //     uint128 amount0Requested,
    //     uint128 amount1Requested
    // ) {
    //     // we don't need to checkTicks here, because invalid positions will never have non-zero tokensOwed{0,1}
    //     Position &position = positions.get(msg.sender, tickLower, tickUpper);

    //     uint128 amount0 = amount0Requested > position.tokensOwed0 ? position.tokensOwed0 : amount0Requested;
    //     uint128 amount1 = amount1Requested > position.tokensOwed1 ? position.tokensOwed1 : amount1Requested;

    //     if (amount0 > 0) {
    //         position.tokensOwed0 -= amount0;
    //         safeTransfer(token0, recipient, amount0);
    //     }
    //     if (amount1 > 0) {
    //         position.tokensOwed1 -= amount1;
    //         safeTransfer(token1, recipient, amount1);
    //     }

    //     return std::pair(amount0, amount1);
    //     emit Collect(msg.sender, recipient, tickLower, tickUpper, amount0, amount1);
    // }
};

#endif