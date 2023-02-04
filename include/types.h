#ifndef headerfiletypes
#define headerfiletypes

#include <string>

#include "../lib/ttmath/ttmathint.h"
#include "../lib/ttmath/ttmathuint.h"

typedef unsigned int uint;
typedef uint uint8;
typedef uint uint16;
typedef uint uint24;
typedef uint uint32;
typedef ttmath::UInt<2>   uint64;
typedef ttmath::UInt<4>   uint128;
typedef ttmath::UInt<5>   uint160;
typedef ttmath::UInt<8>   uint256;
typedef ttmath::UInt<16>  uint512;
typedef ttmath::UInt<32>  address;
typedef int int16;
typedef int int24;
typedef ttmath::Int<2> int56;
typedef ttmath::Int<4> int128;
typedef ttmath::Int<8> int256;

typedef std::string bytes32;

struct Slot0 {
    // the current price
    uint160 sqrtPriceX96;
    // the current tick
    int24 tick;
    // the most-recently updated index of the observations array
    // uint16 observationIndex;
    // the current maximum number of observations that are being stored
    // uint16 observationCardinality;
    // the next maximum number of observations to store, triggered in observations.write
    // uint16 observationCardinalityNext;
    // the current protocol fee as a percentage of the swap fee taken on withdrawal
    // represented as an integer denominator (1/x)%
    // uint8 feeProtocol;
    Slot0() { sqrtPriceX96 = 0; }
    Slot0(
        uint160 sqrtPriceX96,
        int24 tick
        // uint16 observationIndex,
        // uint16 observationCardinality,
        // uint16 observationCardinalityNext,
        // uint8 feeProtocol
    ) : sqrtPriceX96(sqrtPriceX96),
        tick(tick)
        // observationIndex(observationIndex),
        // observationCardinality(observationCardinality),
        // observationCardinalityNext(observationCardinalityNext),
        // feeProtocol(feeProtocol)
        {

    }
    friend std::istream& operator>>(std::istream& is, Slot0& slot) {
        is >> slot.sqrtPriceX96 >> slot.tick;
            // >> slot.observationIndex >> slot.observationCardinality
            // >> slot.observationCardinalityNext >> slot.feeProtocol;
        return is;
    }
    friend std::ostream& operator<<(std::ostream& os, const Slot0& slot) {
        os << slot.sqrtPriceX96 << " " << slot.tick;
            // << slot.observationIndex << " " << slot.observationCardinality << " "
            // << slot.observationCardinalityNext << " " << slot.feeProtocol;
        return os;
    }
    void print() {
        std::cout << "---------- Slot0 INFO BELOW ------------" << std::endl
            << "sqrtPriceX96: " << sqrtPriceX96
            << "\ntick: " << tick
            // << "\nobservationIndex: " << observationIndex
            // << "\nobservationCardinality: " << observationCardinality
            // << "\nobservationCardinalityNext: " << observationCardinalityNext
            // << "\nfeeProtocol: " << feeProtocol
            << std::endl;
        std::cout << "---------- Slot0 INFO ABOVE ------------" << std::endl;
    }
};

struct SwapCache {
    // the protocol fee for the input token
    // uint8 feeProtocol;
    // liquidity at the beginning of the swap
    uint128 liquidityStart;
    // the timestamp of the current block
    // uint32 blockTimestamp;
    // the current value of the tick accumulator, computed only if we cross an initialized tick
    // int56 tickCumulative;
    // the current value of seconds per liquidity accumulator, computed only if we cross an initialized tick
    // uint160 secondsPerLiquidityCumulativeX128;
    // whether we've computed and cached the above two accumulators
    // bool computedLatestObservation;

    SwapCache(
        // uint8 feeProtocol,
        uint128 liquidityStart
        // uint32 blockTimestamp,
        // int56 tickCumulative,
        // uint160 secondsPerLiquidityCumulativeX128,
        // bool computedLatestObservation
    ) : // feeProtocol(feeProtocol),
        liquidityStart(liquidityStart)
        // blockTimestamp(blockTimestamp),
        // tickCumulative(tickCumulative),
        // secondsPerLiquidityCumulativeX128(secondsPerLiquidityCumulativeX128),
        // computedLatestObservation(computedLatestObservation)
        {

    }
};

    // the top level state of the swap, the results of which are recorded in storage at the end
struct SwapState {
    // the amount remaining to be swapped in/out of the input/output asset
    int256 amountSpecifiedRemaining;
    // the amount already swapped out/in of the output/input asset
    int256 amountCalculated;
    // current sqrt(price)
    uint160 sqrtPriceX96;
    // the tick associated with the current price
    int24 tick;
    // the global fee growth of the input token
    // uint256 feeGrowthGlobalX128;
    // amount of input token paid as protocol fee
    // uint128 protocolFee;
    // the current liquidity in range
    uint128 liquidity;

    SwapState(
        int256 amountSpecifiedRemaining,
        int256 amountCalculated,
        uint160 sqrtPriceX96,
        int24 tick,
        // uint256 feeGrowthGlobalX128,
        // uint128 protocolFee,
        uint128 liquidity
    ) : amountSpecifiedRemaining(amountSpecifiedRemaining),
        amountCalculated(amountCalculated),
        sqrtPriceX96(sqrtPriceX96),
        tick(tick),
        // feeGrowthGlobalX128(feeGrowthGlobalX128),
        // protocolFee(protocolFee),
        liquidity(liquidity)
    {

    }
};

struct StepComputations {
    // the price at the beginning of the step
    uint160 sqrtPriceStartX96;
    // the next tick to swap to from the current tick in the swap direction
    int24 tickNext;
    // whether tickNext is initialized or not
    bool initialized;
    // sqrt(price) for the next tick (1/0)
    uint160 sqrtPriceNextX96;
    // how much is being swapped in in this step
    uint256 amountIn;
    // how much is being swapped out
    uint256 amountOut;
    // how much fee is being paid in
    uint256 feeAmount;
};

struct Block
{
    uint256 basefee;
    uint256 chainid;
    uint256 coinbase;
    uint256 difficulty;
    uint256 gaslimit;
    uint256 number;
    uint32 timestamp;
};

struct Message
{
    bytes32 data;
    address sender;
    bytes32 sig;
    uint256 value;
};

struct ModifyPositionParams {
    // the address that owns the position
    address owner;
    // the lower and upper tick of the position
    int24 tickLower;
    int24 tickUpper;
    // any change in liquidity
    int128 liquidityDelta;
    ModifyPositionParams(
        address owner,
        int24 tickLower,
        int24 tickUpper,
        int128 liquidityDelta
    ) : owner(owner),
        tickLower(tickLower),
        tickUpper(tickUpper),
        liquidityDelta(liquidityDelta) {

    }
    void print() {
        std::cout << owner << " " << tickLower << " " << tickUpper << " " << liquidityDelta << std::endl;
    }
};


#endif