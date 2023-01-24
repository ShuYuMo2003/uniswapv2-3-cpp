#ifndef headerfileoracle
#define headerfileoracle

#include <map>

#include "types.h"

struct Observation {
    // the block timestamp of the observation
    uint32 blockTimestamp;
    // the tick accumulator, i.e. tick * time elapsed since the pool was first initialized
    int56 tickCumulative;
    // the seconds per liquidity, i.e. seconds elapsed / max(1, liquidity) since the pool was first initialized
    uint160 secondsPerLiquidityCumulativeX128;
    // whether or not the observation is initialized
    bool initialized;
    Observation() {}
    Observation(
        uint32 blockTimestamp,
        int56 tickCumulative,
        uint160 secondsPerLiquidityCumulativeX128,
        bool initialized
    ) : blockTimestamp(blockTimestamp),
        tickCumulative(tickCumulative),
        secondsPerLiquidityCumulativeX128(secondsPerLiquidityCumulativeX128),
        initialized(initialized) {

    }
};

struct Observations {
    std::map<int, Observation> data;

    /// @notice Initialize the oracle array by writing the first slot. Called once for the lifecycle of the observations array
    /// @param self The stored oracle array
    /// @param time The time of the oracle initialization, via block.timestamp truncated to uint32
    /// @return cardinality The number of populated elements in the oracle array
    /// @return cardinalityNext The new length of the oracle array, independent of population
    std::pair<uint16, uint16> initialize(uint32 time) {
        data[0] = Observation(time, 0, 0, true);
        return std::make_pair(1, 1);
    }

    std::pair<int56, uint160> observeSingle(
        uint32 time,
        uint32 secondsAgo,
        int24 tick,
        uint16 index,
        uint128 liquidity,
        uint16 cardinality
    ) //internal view returns (int56 tickCumulative, uint160 secondsPerLiquidityCumulativeX128) {
    {
        return std::make_pair(int56(0), uint160(0));
        // if (secondsAgo == 0) {
        //     Observation memory last = data[index];
        //     if (last.blockTimestamp != time) last = transform(last, time, tick, liquidity);
        //     return (last.tickCumulative, last.secondsPerLiquidityCumulativeX128);
        // }

        // uint32 target = time - secondsAgo;

        // (Observation memory beforeOrAt, Observation memory atOrAfter) = getSurroundingObservations(
        //     data,
        //     time,
        //     target,
        //     tick,
        //     index,
        //     liquidity,
        //     cardinality
        // );

        // if (target == beforeOrAt.blockTimestamp) {
        //     // we're at the left boundary
        //     return (beforeOrAt.tickCumulative, beforeOrAt.secondsPerLiquidityCumulativeX128);
        // } else if (target == atOrAfter.blockTimestamp) {
        //     // we're at the right boundary
        //     return (atOrAfter.tickCumulative, atOrAfter.secondsPerLiquidityCumulativeX128);
        // } else {
        //     // we're in the middle
        //     uint32 observationTimeDelta = atOrAfter.blockTimestamp - beforeOrAt.blockTimestamp;
        //     uint32 targetDelta = target - beforeOrAt.blockTimestamp;
        //     return (
        //         beforeOrAt.tickCumulative +
        //             ((atOrAfter.tickCumulative - beforeOrAt.tickCumulative) / observationTimeDelta) *
        //             targetDelta,
        //         beforeOrAt.secondsPerLiquidityCumulativeX128 +
        //             uint160(
        //                 (uint256(
        //                     atOrAfter.secondsPerLiquidityCumulativeX128 - beforeOrAt.secondsPerLiquidityCumulativeX128
        //                 ) * targetDelta) / observationTimeDelta
        //             )
        //     );
        // }
    }
    /// @notice Writes an oracle observation to the array
    /// @dev Writable at most once per block. Index represents the most recently written element. cardinality and index must be tracked externally.
    /// If the index is at the end of the allowable array length (according to cardinality), and the next cardinality
    /// is greater than the current one, cardinality may be increased. This restriction is created to preserve ordering.
    /// @param self The stored oracle array
    /// @param index The index of the observation that was most recently written to the observations array
    /// @param blockTimestamp The timestamp of the new observation
    /// @param tick The active tick at the time of the new observation
    /// @param liquidity The total in-range liquidity at the time of the new observation
    /// @param cardinality The number of populated elements in the oracle array
    /// @param cardinalityNext The new length of the oracle array, independent of population
    /// @return indexUpdated The new index of the most recently written element in the oracle array
    /// @return cardinalityUpdated The new cardinality of the oracle array
    std::pair<uint16, uint16> write(
        uint16 index,
        uint32 blockTimestamp,
        int24 tick,
        uint128 liquidity,
        uint16 cardinality,
        uint16 cardinalityNext
    ) {
        return std::make_pair(index, cardinality);
        // Observation last = data[index];

        // // early return if we've already written an observation this block
        // if (last.blockTimestamp == blockTimestamp) return std::make_pair(index, cardinality);

        // uint16 cardinalityUpdated;
        // // if the conditions are right, we can bump the cardinality
        // if (cardinalityNext > cardinality && index == (cardinality - 1)) {
        //     cardinalityUpdated = cardinalityNext;
        // } else {
        //     cardinalityUpdated = cardinality;
        // }

        // uint16 indexUpdated = (index + 1) % cardinalityUpdated;
        // data[indexUpdated] = transform(last, blockTimestamp, tick, liquidity);
        // return std::make_pair(indexUpdated, cardinalityUpdated);
    }
};

#endif