#ifndef headerfileconsts
#define headerfileconsts

#include "types.h"

const uint160 MIN_SQRT_RATIO = uint160("4295128739");
const uint160 MAX_SQRT_RATIO = uint160("1461446703485210103287273052203988822378723970342");
// FixedPoint96.sol
const uint8 RESOLUTION = 96;
// 2^96
const uint256 Q96 = uint256("79228162514264337593543950336");
// 2^128
const uint256 Q128 = uint256("340282366920938463463374607431768211456");

#endif