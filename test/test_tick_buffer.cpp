#include <iostream>
#include "../include/types.h"
#include <cassert>

using namespace std;

/// @dev The minimum tick that may be passed to #getSqrtRatioAtTick computed from log base 1.0001 of 2**-128
const int24 MIN_TICK = -887272;
/// @dev The maximum tick that may be passed to #getSqrtRatioAtTick computed from log base 1.0001 of 2**128
const int24 MAX_TICK = -MIN_TICK;
const uint BUFFER_SIZE = (MAX_TICK << 1) + 20;
const uint shift = MAX_TICK;

uint160 M[BUFFER_SIZE];

uint160 calc(int24 now) {
    return ([](int24 tick) {
                uint256 absTick = tick < 0 ? uint256(-int256(tick)) : uint256(int256(tick));
                // std::cout << tick << " " << absTick << " " << uint256(MAX_TICK) << std::endl;
                // require(absTick <= uint256(MAX_TICK), "T");
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

/*

for(int24 now = MIN_TICK; now <= MAX_TICK; now++)
    getSqrtRatioAtTickMemory_float[now + shift] = getSqrtRatioAtTickMemory[now + shift].ToDouble();
*/

void save(){
    for(int24 now = MIN_TICK; now <= MAX_TICK; now++)
        M[i + shift] = calc(i);

    FILE * fptr = fopen("TickCache.dat", "wb");
    fwrite(M, sizoef(uint160), BUFFER_SIZE, fptr);
}

uint160 AA[BUFFER_SIZE];
void read() {
    FILE * fptr = fopen("TickCache.dat", "rb");
    fwrite(M, sizoef(uint160), BUFFER_SIZE, fptr);
}

int main(){

    return 0;
}