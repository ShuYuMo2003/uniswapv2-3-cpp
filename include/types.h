#ifndef headerfiletypes
#define headerfiletypes

#include <string>

#include "../lib/ttmath/ttmathint.h"
#include "../lib/ttmath/ttmathuint.h"

#define FloatType double

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

template<bool enable_float>
struct Slot0 {
    // the current price
    typename std::conditional<enable_float, FloatType, uint160>::type sqrtPriceX96;
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
        typename std::conditional<enable_float, FloatType, uint160>::type sqrtPriceX96,
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


// the top level state of the swap, the results of which are recorded in storage at the end
template<bool enable_float>
struct SwapState {
    // the amount remaining to be swapped in/out of the input/output asset
    typename std::conditional<enable_float, FloatType, int256>::type amountSpecifiedRemaining;
    // the amount already swapped out/in of the output/input asset
    typename std::conditional<enable_float, FloatType, int256>::type amountCalculated;
    // current sqrt(price)
    typename std::conditional<enable_float, FloatType, uint160>::type sqrtPriceX96;
    // the tick associated with the current price
    int24 tick;
    // the current liquidity in range
    typename std::conditional<enable_float, FloatType, uint128>::type liquidity;

    SwapState(
        typename std::conditional<enable_float, FloatType, int256>::type  amountSpecifiedRemaining,
        typename std::conditional<enable_float, FloatType, int256>::type  amountCalculated,
        typename std::conditional<enable_float, FloatType, uint160>::type sqrtPriceX96,
        int24 tick,
        typename std::conditional<enable_float, FloatType, uint128>::type liquidity
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

template<bool enable_float>
struct StepComputations {
    // the price at the beginning of the step
    typename std::conditional<enable_float, FloatType, uint160>::type sqrtPriceStartX96;
    // the next tick to swap to from the current tick in the swap direction
    int24 tickNext;
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


template<bool enable_float>
struct ModifyPositionParams {
    // the address that owns the position
    address owner;
    // the lower and upper tick of the position
    int24 tickLower;
    int24 tickUpper;
    // any change in liquidity
    typename std::conditional<enable_float, FloatType, int128>::type liquidityDelta;
    ModifyPositionParams(
        address owner,
        int24 tickLower,
        int24 tickUpper,
        typename std::conditional<enable_float, FloatType, int128>::type liquidityDelta
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