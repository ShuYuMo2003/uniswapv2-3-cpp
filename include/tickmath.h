#ifndef headerfiletickmath
#define headerfiletickmath

#include "consts.h"
#include "types.h"
#include "util.h"
#include <cassert>
#include <algorithm>

/// @dev The minimum tick that may be passed to #getSqrtRatioAtTick computed from log base 1.0001 of 2**-128
const int24 MIN_TICK = -887272;
/// @dev The maximum tick that may be passed to #getSqrtRatioAtTick computed from log base 1.0001 of 2**128
const int24 MAX_TICK = -MIN_TICK;


const uint BUFFER_SIZE = (MAX_TICK << 1) + 20;
const uint shift = MAX_TICK;
bool ticks_price_initialized = false;
uint160 getSqrtRatioAtTickMemory[BUFFER_SIZE];
double getSqrtRatioAtTickMemory_float[BUFFER_SIZE];
void initializeTicksPrice() {
    FILE * TickCacheFileHandle = fopen("TickCache.dat", "rb");
    if(TickCacheFileHandle != NULL) {
        fread(getSqrtRatioAtTickMemory, sizeof(uint160), BUFFER_SIZE, TickCacheFileHandle);
    } else {
        for(int24 now = MIN_TICK; now <= MAX_TICK; now++) {
            getSqrtRatioAtTickMemory[now + shift] = ([](int24 tick) {
                uint256 absTick = tick < 0 ? uint256(-int256(tick)) : uint256(int256(tick));
                // std::cout << tick << " " << absTick << " " << uint256(MAX_TICK) << std::endl;
                require(absTick <= uint256(MAX_TICK), "T");
                uint256 ratio = uint256((absTick & 1) != 0 ? "340265354078544963557816517032075149313" : "340282366920938463463374607431768211456");
                if ((absTick&2) != 0) ratio = (ratio * uint256("340248342086729790484326174814286782778")) >> 128;
                if ((absTick&4) != 0) ratio = (ratio * uint256("340214320654664324051920982716015181260")) >> 128;
                if ((absTick&8) != 0) ratio = (ratio * uint256("340146287995602323631171512101879684304")) >> 128;
                if ((absTick&16) != 0) ratio = (ratio * uint256("340010263488231146823593991679159461444")) >> 128;
                if ((absTick&32) != 0) ratio = (ratio * uint256("339738377640345403697157401104375502016")) >> 128;
                if ((absTick&64) != 0) ratio = (ratio * uint256("339195258003219555707034227454543997025")) >> 128;
                if ((absTick&128) != 0) ratio = (ratio * uint256("338111622100601834656805679988414885971")) >> 128;
                if ((absTick&256) != 0) ratio = (ratio * uint256("335954724994790223023589805789778977700")) >> 128;
                if ((absTick&512) != 0) ratio = (ratio * uint256("331682121138379247127172139078559817300")) >> 128;
                if ((absTick&1024) != 0) ratio = (ratio * uint256("323299236684853023288211250268160618739")) >> 128;
                if ((absTick&2048) != 0) ratio = (ratio * uint256("307163716377032989948697243942600083929")) >> 128;
                if ((absTick&4096) != 0) ratio = (ratio * uint256("277268403626896220162999269216087595045")) >> 128;
                if ((absTick&8192) != 0) ratio = (ratio * uint256("225923453940442621947126027127485391333")) >> 128;
                if ((absTick&16384) != 0) ratio = (ratio * uint256("149997214084966997727330242082538205943")) >> 128;
                if ((absTick&32768) != 0) ratio = (ratio * uint256("66119101136024775622716233608466517926")) >> 128;
                if ((absTick&(uint256(1)<<16)) != 0) ratio = (ratio * uint256("12847376061809297530290974190478138313")) >> 128;
                if ((absTick&(uint256(1)<<17)) != 0) ratio = (ratio * uint256("485053260817066172746253684029974020")) >> 128;
                if ((absTick&(uint256(1)<<18)) != 0) ratio = (ratio * uint256("691415978906521570653435304214168")) >> 128;
                if ((absTick&(uint256(1)<<19)) != 0) ratio = (ratio * uint256("1404880482679654955896180642")) >> 128;

                if (tick > 0) ratio = uint256("115792089237316195423570985008687907853269984665640564039457584007913129639935") / ratio;

                return uint160((ratio>>32) + (ratio % (uint256(1)<<32) == 0 ? 0 : 1));
            }) (now);
        }
        FILE * newTickCacheFileHandle = fopen("TickCache.dat", "wb");
        assert(newTickCacheFileHandle != NULL);
        fwrite(getSqrtRatioAtTickMemory, sizeof(uint160), BUFFER_SIZE, newTickCacheFileHandle);
    }

    for(int24 now = MIN_TICK; now <= MAX_TICK; now++)
        getSqrtRatioAtTickMemory_float[now + shift] = getSqrtRatioAtTickMemory[now + shift].X96ToDouble();

    ticks_price_initialized = true;
}


/// @notice Calculates sqrt(1.0001^tick) * 2^96
/// @dev Throws if |tick| > max tick
/// @param tick The input tick for the above formula
/// @return sqrtPriceX96 A Fixed point Q64.96 number representing the sqrt of the ratio of the two assets (token1/token0)
/// at the given tick
uint160 getSqrtRatioAtTick(int24 tick) {
    require(ticks_price_initialized, "You should call `initializeTicksPrice()` in tickmath.h to initilize the price of tick.");
    require(MIN_TICK <= tick && tick <= MAX_TICK);
    return getSqrtRatioAtTickMemory[tick + shift];
}

/// @notice Calculates the greatest tick value such that getRatioAtTick(tick) <= ratio
/// @dev Throws in case sqrtPriceX96 < MIN_SQRT_RATIO, as MIN_SQRT_RATIO is the lowest value getRatioAtTick may
/// ever return.
/// @param sqrtPriceX96 The sqrt ratio for which to compute the tick as a Q64.96
/// @return tick The greatest tick for which the ratio is less than or equal to the input ratio
int24 getTickAtSqrtRatio_float(double sqrtPriceX96) {
    require(ticks_price_initialized, "You should call `initializeTicksPrice()` in tickmath.h to initilize the price of tick.");
    return (std::upper_bound(getSqrtRatioAtTickMemory_float,
                        getSqrtRatioAtTickMemory_float + (MAX_TICK << 1 | 1) + 1,
                        sqrtPriceX96)
            - getSqrtRatioAtTickMemory_float - 1) - shift;
}

int24 getTickAtSqrtRatio(uint160 sqrtPriceX96) {
    require(ticks_price_initialized, "You should call `initializeTicksPrice()` in tickmath.h to initilize the price of tick.");
    return (std::upper_bound(getSqrtRatioAtTickMemory,
                        getSqrtRatioAtTickMemory + (MAX_TICK << 1 | 1) + 1,
                        sqrtPriceX96)
            - getSqrtRatioAtTickMemory - 1) - shift;
/*
    // second inequality must be < because the price can never reach the price at the max tick
    require(sqrtPriceX96 >= MIN_SQRT_RATIO && sqrtPriceX96 < MAX_SQRT_RATIO, "R");
    uint256 ratio = uint256(sqrtPriceX96) << 32;

    uint256 r = ratio;
    uint msb = 0;

    for (int i = 7; ~i; --i) {
        uint f = r > (uint256(1) << (1<<i)) - 1;
        f = f << i;
        msb |= f;
        r = r >> f;
    }

    if (msb >= 128) r = ratio >> (msb - 127);
    else r = ratio << (127 - msb);

    int256 log_2 = (int256(msb) - 128) << 64;

    for (int i = 63; i >= 50; --i) {
        r = (r * r) >> 127;
        uint256 f = r >> 128;
        log_2 = log_2 | (f << i);
        r = r >> f.ToUInt();
    }

    int256 log_sqrt10001 = log_2 * uint256("255738958999603826347141"); // 128.128 number

    int24 tickLow = ((log_sqrt10001 - int256("3402992956809132418596140100660247210")) >> 128).ToUInt();
    int24 tickHi = ((log_sqrt10001 + int256("291339464771989622907027621153398088495")) >> 128).ToUInt();
    tickLow &= (1<<24) - 1, tickHi &= (1<<24) - 1;
    // std::cout << tickLow << " " << tickHi << std::endl;
    if (tickLow >= (1<<23)) tickLow = -((1<<24) - tickLow);
    if (tickHi >= (1<<23)) tickHi = -((1<<24) - tickHi);


    // std::cout << "------ " << tickHi << " " << sqrtPriceX96 << " " << tickLow << std::endl;
    // std::cout << "====== " << getSqrtRatioAtTick(tickHi) << std::endl;
    // std::cout << "====== " << sqrtPriceX96 << std::endl;

    return tickLow == tickHi ? tickLow : getSqrtRatioAtTick(tickHi) <= sqrtPriceX96 ? tickHi : tickLow;
*/
}

#endif