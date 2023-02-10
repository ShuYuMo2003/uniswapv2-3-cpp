#ifndef headerfiletick
#define headerfiletick

#include <unordered_map>

#include "types.h"
#include "liquiditymath.h"

template<bool enable_float>
struct Tick {
    // the total position liquidity that references this tick
    typename std::conditional<enable_float, FloatType, uint128>::type liquidityGross;
    // amount of net liquidity added (subtracted) when tick is crossed from left to right (right to left),
    typename std::conditional<enable_float, FloatType, int128>::type  liquidityNet;
    // // the seconds per unit of liquidity on the _other_ side of this tick (relative to the current tick)
    // // only has relative meaning, not absolute — the value depends on when the tick is initialized
    // uint160 secondsPerLiquidityOutsideX128;
    // the seconds spent on the other side of the tick (relative to the current tick)
    // only has relative meaning, not absolute — the value depends on when the tick is initialized
    // uint32 secondsOutside;
    // true iff the tick is initialized, i.e. the value is exactly equivalent to the expression liquidityGross != 0
    // these 8 bits are set to prevent fresh sstores when crossing newly initialized ticks
    bool initialized;
    Tick() { liquidityGross = 0, liquidityNet = 0, initialized = 0; }
    friend std::istream& operator>>(std::istream& is, Tick& tick) {
        is >> tick.liquidityGross >> tick.liquidityNet
            >> tick.initialized;
        return is;
    }
    friend std::ostream& operator<<(std::ostream& os, const Tick& tick) {
        os << tick.liquidityGross << " " << tick.liquidityNet << " "
            << tick.initialized;
        return os;
    }
};


template<bool enable_float>
struct Ticks {
    std::unordered_map<int24, Tick<enable_float>> data;
    /// @notice Updates a tick and returns true if the tick was flipped from initialized to uninitialized, or vice versa
    /// @param self The mapping containing all tick information for initialized ticks
    /// @param tick The tick that will be updated
    /// @param tickCurrent The current tick
    /// @param liquidityDelta A new amount of liquidity to be added (subtracted) when tick is crossed from left to right (right to left)
    /// @param feeGrowthGlobal0X128 The all-time global fee growth, per unit of liquidity, in token0
    /// @param feeGrowthGlobal1X128 The all-time global fee growth, per unit of liquidity, in token1
    /// @param secondsPerLiquidityCumulativeX128 The all-time seconds per max(1, liquidity) of the pool
    /// @param tickCumulative The tick * time elapsed since the pool was first initialized
    /// @param time The current block timestamp cast to a uint32
    /// @param upper true for updating a position's upper tick, or false for updating a position's lower tick
    /// @param maxLiquidity The maximum liquidity allocation for a single tick
    /// @return flipped Whether the tick was flipped from initialized to uninitialized, or vice versa
    bool update(
        int24 tick,
        int24 tickCurrent,
        typename std::conditional<enable_float, FloatType, int128>::type liquidityDelta,
        uint32 time,
        bool upper,
        typename std::conditional<enable_float, FloatType, uint128>::type maxLiquidity
    ) {
        Tick<enable_float> &info = data[tick];

        typename std::conditional<enable_float, FloatType, uint128>::type  \
            liquidityGrossBefore = info.liquidityGross;

        typename std::conditional<enable_float, FloatType, uint128>::type  \
            liquidityGrossAfter = addDelta(liquidityGrossBefore, liquidityDelta);

        require(liquidityGrossAfter <= maxLiquidity, "LO");

        bool flipped = (liquidityGrossAfter == 0) != (liquidityGrossBefore == 0);

        if (liquidityGrossBefore == 0) {
            info.initialized = true;
        }

        info.liquidityGross = liquidityGrossAfter;

        // when the lower (upper) tick is crossed left to right (right to left), liquidity must be added (removed)
        info.liquidityNet = upper
            ? (typename std::conditional<enable_float, FloatType, int256>::type)(info.liquidityNet) - liquidityDelta
            : (typename std::conditional<enable_float, FloatType, int256>::type)(info.liquidityNet) + liquidityDelta;

        return flipped;
    }
    /// @notice Clears tick data
    /// @param self The mapping containing all initialized tick information for initialized ticks
    /// @param tick The tick that will be cleared
    void clear(int24 tick) {
        data.erase(tick);
    }
    typename std::conditional<enable_float, FloatType, int128>::type cross(int24 tick) { return data[tick].liquidityNet; }
    friend std::istream& operator>>(std::istream& is, Ticks& ticks) {
        ticks.data.clear();
        int num;
        is >> num;
        for (int i = 0; i < num; ++i) {
            int k = 0; is >> k >> ticks.data[k];
        }
        return is;
    }
    friend std::ostream& operator<<(std::ostream& os, const Ticks &ticks) {
        os << ticks.data.size() << std::endl;
        for (auto [k, v] : ticks.data) {
            os << k << " " << v << std::endl;
        }
        return os;
    }
};

#endif