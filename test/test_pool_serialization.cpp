#include "../include/pool.h"
#include <iostream>
#include <vector>

using namespace std;

/*
    uint24 fee;
    int24 tickSpacing;
    LiquidityType maxLiquidityPerTick;
    LiquidityType liquidity;
    Slot0<enable_float> slot0;
    Ticks<enable_float> ticks;
    TickBitMapBaseOnVector tickBitmap;
*/
template<bool pool_type>
void validate() {
    static char buffer[100000];
    Pool<pool_type> pool0("pool_state");
    size_t length0 = DumpPool(&pool0, buffer);

    Pool<pool_type> pool1;
    size_t length1 = LoadPool(&pool1, buffer);

    assert(length1 == length0);
    assert(pool0.fee == pool1.fee);
    assert(pool0.tickSpacing == pool1.tickSpacing);
    assert(pool0.maxLiquidityPerTick == pool1.maxLiquidityPerTick);
    assert(pool0.liquidity == pool1.liquidity);
    assert(pool0.slot0.tick == pool1.slot0.tick);
    assert(pool0.slot0.sqrtPriceX96 == pool1.slot0.sqrtPriceX96);
    assert(pool0.ticks.data.size() == pool1.ticks.data.size());

    vector< pair< int24, Tick<pool_type> > > ticks0;
    ticks0.clear();
    for(auto kv : pool0.ticks.data)
        ticks0.push_back(kv);
    sort(ticks0.begin(), ticks0.end(), [](pair<int24, Tick<pool_type> > lhs,
                                          pair<int24, Tick<pool_type> > rhs)
                                            { return lhs.first < rhs.first; } );

    vector< pair< int24, Tick<pool_type> > > ticks1;
    ticks1.clear();
    for(auto kv : pool1.ticks.data)
        ticks1.push_back(kv);
    sort(ticks1.begin(), ticks1.end(), [](pair<int24, Tick<pool_type> > lhs,
                                          pair<int24, Tick<pool_type> > rhs)
                                            { return lhs.first < rhs.first; } );

    for(int i = 0; i < ticks1.size(); i++) {
        assert(ticks0[i].first == ticks1[i].first);
        assert(ticks0[i].second.liquidityGross == ticks1[i].second.liquidityGross);
        assert(ticks0[i].second.liquidityNet == ticks1[i].second.liquidityNet);
        assert(ticks0[i].second.initialized == ticks1[i].second.initialized);
    }

    assert(pool0.tickBitmap.data.size() == pool1.tickBitmap.data.size());
    for(int i = 0; i < pool0.tickBitmap.data.size(); i++)
        assert(pool0.tickBitmap.data[i] == pool1.tickBitmap.data[i]);
}

int main(){
    validate<true>();
    cerr << "validate pool true" << endl;
    validate<false>();
    cerr << "validate pool false" << endl;
    return 0;
}