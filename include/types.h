#ifndef headerfiletypes
#define headerfiletypes

#include <string>
#include <iostream>

#include "../lib/ttmath/ttmathint.h"
#include "../lib/ttmath/ttmathuint.h"

#define FloatType double

// std::istream& operator>>(std::istream& is, __float128 & x) {
//     double t;
//     is>>t;
//     x = t;
//     return is;
// }
// std::ostream& operator<<(std::ostream& os, const __float128 & x) {
//     double t = x;
//     os << x;
//     return os;
// }

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
    typename std::conditional<enable_float, FloatType, uint160>::type sqrtPriceX96;
    int24 tick;
    Slot0() { sqrtPriceX96 = 0; }
    Slot0(
        typename std::conditional<enable_float, FloatType, uint160>::type sqrtPriceX96,
        int24 tick
    ) : sqrtPriceX96(sqrtPriceX96),
        tick(tick)
        {
    }
    friend std::istream& operator>>(std::istream& is, Slot0& slot) {
        is >> slot.sqrtPriceX96 >> slot.tick;
        return is;
    }
    friend std::ostream& operator<<(std::ostream& os, const Slot0& slot) {
        os << slot.sqrtPriceX96 << " " << slot.tick;
        return os;
    }
    void print() {
        std::cout << "---------- Slot0 INFO BELOW ------------" << std::endl
            << "sqrtPriceX96: " << sqrtPriceX96
            << "\ntick: " << tick
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
    // the lower and upper tick of the position
    int24 tickLower;
    int24 tickUpper;
    // any change in liquidity
    typename std::conditional<enable_float, FloatType, int128>::type liquidityDelta;
    ModifyPositionParams(
        int24 tickLower,
        int24 tickUpper,
        typename std::conditional<enable_float, FloatType, int128>::type liquidityDelta
    ) : tickLower(tickLower),
        tickUpper(tickUpper),
        liquidityDelta(liquidityDelta) {

    }
    void print() {
        std::cout << tickLower << " " << tickUpper << " " << liquidityDelta;
    }
};


#endif