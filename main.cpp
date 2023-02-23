#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "include/types.h"
#include "include/pool.h"
struct SwapEvent{
    bool zeroToOne;
    int256 amount;
// for validating.
    uint160 sqrtPrice;
    int256 ramount0;
    int256 ramount1;
    uint128 liquidity;
    int tick;
};

struct MBEvent{
    int tickLower;
    int tickUpper;
    uint128 amount;
// for validating.
    uint256 ramount0;
    uint256 ramount1;
};

struct InEvent{
    uint160 Price;
// for validating.
    int tick;
};

int main(){
    std::cout << sizeof(SwapEvent) << " " << sizeof(MBEvent) << " " << sizeof(InEvent) << std::endl;
    return 0;
}