#ifndef headerfiletick
#define headerfiletick

#include <unordered_map>

#include "types.h"
#include "liquiditymath.h"

/// @dev The minimum tick that may be passed to #getSqrtRatioAtTick computed from log base 1.0001 of 2**-128
const int24 MIN_TICK = -887272;
/// @dev The maximum tick that may be passed to #getSqrtRatioAtTick computed from log base 1.0001 of 2**128
const int24 MAX_TICK = -MIN_TICK;

template<bool enable_float>
struct _Tick {
    int24 id;
    typename std::conditional<enable_float, FloatType, uint128>::type liquidityGross;
    typename std::conditional<enable_float, FloatType, int128>::type  liquidityNet;
    _Tick() { id = 0; liquidityGross = 0, liquidityNet = 0; }
    _Tick(int _id) { id = _id; liquidityGross = 0, liquidityNet = 0; }
};

_Tick<true> MAX_TICK_OBJ_TRUE = _Tick<true>(MAX_TICK);
_Tick<false> MAX_TICK_OBJ_FALSE = _Tick<false>(MAX_TICK);

_Tick<true> MIN_TICK_OBJ_TRUE = _Tick<true>(MIN_TICK);
_Tick<false> MIN_TICK_OBJ_FALSE = _Tick<false>(MIN_TICK);

template<bool enable_float>
bool updateTick(
    _Tick<enable_float> * tick,
    typename std::conditional<enable_float, FloatType, int128>::type liquidityDelta,
    bool upper,
    typename std::conditional<enable_float, FloatType, uint128>::type maxLiquidity
) {
    // std::cerr << "updateTick t = " << tick->id << std::endl;
    typename std::conditional<enable_float, FloatType, uint128>::type  \
        liquidityGrossBefore = tick->liquidityGross;

    typename std::conditional<enable_float, FloatType, uint128>::type  \
        liquidityGrossAfter = addDelta(liquidityGrossBefore, liquidityDelta);

    // std::cerr << "liqc = " << liquidityGrossBefore << " " << liquidityGrossAfter << std::endl;

    require(liquidityGrossAfter <= maxLiquidity, "LO");

    bool erase = isZero(liquidityGrossAfter) && !isZero(liquidityGrossBefore);
    // std::cerr << "erase: " << erase << std::endl;

    tick->liquidityGross = liquidityGrossAfter;

    // when the lower (upper) tick is crossed left to right (right to left), liquidity must be added (removed)
    tick->liquidityNet = upper
        ? (typename std::conditional<enable_float, FloatType, int256>::type)(tick->liquidityNet) - liquidityDelta
        : (typename std::conditional<enable_float, FloatType, int256>::type)(tick->liquidityNet) + liquidityDelta;

    // printf("tick address = %p\n", tick);
    return erase;
}
template<bool enable_float>
std::istream& operator>>(std::istream& is, _Tick<enable_float>& tick) {
    is >> tick.id >> tick.liquidityGross >> tick.liquidityNet;
    return is;
}

template<bool enable_float>
std::ostream& operator<<(std::ostream& os, const _Tick<enable_float>& tick) {
    os << tick.id << " " << tick.liquidityGross << " " << tick.liquidityNet;
    return os;
}



#endif