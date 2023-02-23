#include <cstdio>
#include <cstdlib>
#include <vector>
#include <ctime>

#include "include/types.h"
#include "include/pool.h"

char buffer[1024 * 1024];

namespace v3{


enum EventType {SWAP, MINT, BURN, INIT};


struct V3Event{
    int type; // Events Type.
    bool zeroToOne;
    int tick;
    int tickLower;
    int tickUpper;
    int256 amount;

    uint160 sqrtPrice;
    int256 ramount0;
    int256 ramount1;
    uint128 liquidity;
};


struct V3Pool{
    Pool<false> * IntPool;
    size_t IntPoolSize;
    Pool<true>  * FloatPool;
    size_t FloatPoolSize;

    void * mallocPool(size_t size) {
        return malloc(size);
    }
    void freePool(void * o) {
        free(o);
    }

    void sync() {
        GenerateFloatPool(IntPool, (Pool<true>*)buffer);
        size_t PoolSize = sizeOfPool((Pool<true>*)buffer);
        if(FloatPool == NULL) {
            FloatPoolSize = PoolSize << 1;
            FloatPool = (Pool<true> *)mallocPool(FloatPoolSize);
        } else {
            if(PoolSize + (sizeof(FloatPool->ticks.temp) << 2) > FloatPoolSize) {
                freePool(FloatPool);
                FloatPoolSize = FloatPoolSize << 1;
                FloatPool = (Pool<true> *)mallocPool(FloatPoolSize);
            }
        }

        memcpy(FloatPool, buffer, PoolSize);
    }



    V3Pool(Pool<false> * o) {
        FloatPoolSize = 0; FloatPool = NULL;
        size_t oldPoolSize = sizeOfPool(o);
        IntPoolSize = oldPoolSize << 1;
        IntPool = (Pool<false> *)mallocPool(IntPoolSize);
        memcpy(IntPool, o, oldPoolSize);
        sync();
    }
    V3Pool(int fee, int tickSpacing, uint256 maxLiquidityPerTick) {
        FloatPoolSize = 0; FloatPool = NULL;
        Pool<false> temppool(fee, tickSpacing, maxLiquidityPerTick);
        size_t poolsize = sizeOfPool(&temppool);
        IntPoolSize = poolsize << 1;
        IntPool = (Pool<false>*)mallocPool(IntPoolSize);
        memcpy(IntPool, &temppool, poolsize);
        sync();
    }

    ~V3Pool(){
        freePool(IntPool);
        freePool(FloatPool);
    }

    void maintainIntPool(){
        size_t PoolSize = sizeOfPool(IntPool);
        if(PoolSize + (sizeof(IntPool->ticks.temp) << 2) > IntPoolSize){
            IntPoolSize <<= 1;
            Pool<false> * newIntPool = (Pool<false> *)mallocPool(IntPoolSize);
            memcpy(newIntPool, IntPool, PoolSize);
            freePool(IntPool);
            IntPool = newIntPool;
        }
    }

    void processEvent(V3Event & e){

        // std::cerr << "Processing" << std::endl;
        int type = e.type;

        static int256 ramount0, ramount1;

        if(type == SWAP) {
            static uint160 SQPRL = uint160("4295128740");
            static uint160 SQPRR = uint160("1461446703485210103287273052203988822378723970341");

            bool success = false;
            memcpy(buffer, IntPool, IntPoolSize);

            // swap case 1.
            if(!success) {
                assert(e.amount != 0);
                std::tie(ramount0, ramount1) = swap(IntPool,
                                                    e.zeroToOne,
                                                    e.amount,
                                                    e.zeroToOne ? SQPRL
                                                                : SQPRR,
                                                    true);

                success = (ramount0 == e.ramount0
                        && ramount1 == e.ramount1
                        && IntPool->liquidity == e.liquidity
                        && IntPool->slot0.tick == e.tick
                        && IntPool->slot0.sqrtPriceX96 == e.sqrtPrice);
            }
            // std::cerr << "QAQ" << std::endl;
            // swap case 2.
            int256 & temp_amount = (e.zeroToOne ? e.ramount1 : e.ramount0);
            if(!success && temp_amount != 0) {
                memcpy(IntPool, buffer, IntPoolSize);
                std::tie(ramount0, ramount1) = swap(IntPool,
                                                    e.zeroToOne,
                                                    temp_amount,
                                                    e.zeroToOne ? SQPRL
                                                                : SQPRR,
                                                    true);

                success = (ramount0 == e.ramount0
                        && ramount1 == e.ramount1
                        && IntPool->liquidity == e.liquidity
                        && IntPool->slot0.tick == e.tick
                        && IntPool->slot0.sqrtPriceX96 == e.sqrtPrice);
            }
            // std::cerr << "QAQ" << std::endl;
            // swap case 3.
            if(!success) {
                memcpy(IntPool, buffer, IntPoolSize);
                std::tie(ramount0, ramount1) = swap(IntPool,
                                                    e.zeroToOne,
                                                    e.amount * 10,
                                                    e.sqrtPrice,
                                                    true);

                success = (ramount0 == e.ramount0
                        && ramount1 == e.ramount1
                        && IntPool->liquidity == e.liquidity
                        && IntPool->slot0.tick == e.tick
                        && IntPool->slot0.sqrtPriceX96 == e.sqrtPrice);
            }

            assert(success);
        } else if(type == MINT) {
            std::tie(ramount0, ramount1) = mint(IntPool,
                                                  e.tickLower,
                                                  e.tickUpper,
                                                  e.amount);
            assert(ramount0 == e.ramount0 && ramount1 == e.ramount1);
            maintainIntPool();
        } else if(type == BURN) {
            std::tie(ramount0, ramount1) = burn(IntPool,
                                                  e.tickLower,
                                                  e.tickUpper,
                                                  e.amount);
            assert(ramount0 == e.ramount0 && ramount1 == e.ramount1);
            maintainIntPool();
        } else if(type == INIT) {
            int nowTick = initialize(IntPool, e.sqrtPrice);
            assert(nowTick == e.tick);
        }
        sync();
    }
};

std::pair<V3Event, bool> tempReadEventsFile(std::istream & is) {
    static char opt[100];

    std::string temp;

    V3Event result;

    if(is >> opt) ; else return std::make_pair(result, false);

    // std::cerr << "Processed " << opt << std::endl;
    is >> temp;
    if(opt[0] == 'i') { // initialize
        result.type = INIT;
        is >> temp;   result.sqrtPrice = temp;
        is >> result.tick;
    }
    if(opt[0] == 's') { // swap
        result.type = SWAP;
        is >> result.zeroToOne;
        is >> temp; result.amount = temp;
        is >> temp; result.sqrtPrice = temp;
        is >> temp; result.ramount0 = temp;
        is >> temp; result.ramount1 = temp;
        is >> temp; result.liquidity = temp;
        is >> result.tick;
    }
    if(opt[0] == 'b' || opt[0] == 'm') { // burn | mint
        result.type = opt[0] == 'b' ? BURN : MINT;
        is >> result.tickLower >> result.tickUpper;
        is >> temp; result.amount = temp;
        is >> temp; result.ramount0 = temp;
        is >> temp; result.ramount1 = temp;
    }
    if(opt[0] == 'c') { // collect
        is >> temp;
        is >> temp;
        is >> temp;
        is >> temp;
    }
    is >> temp;
    return std::make_pair(result, true);
}

}

using v3::V3Pool;
using v3::V3Event;



int main(){
    initializeTicksPrice();
    std::ifstream fin("pool_events_test_");
    int fee; int tickSpacing; uint256 maxLiquidityPerTick;
    fin >> fee >> tickSpacing >> maxLiquidityPerTick;
    std::cerr << "Fee = " << fee << " sp = " << tickSpacing << std::endl;
    V3Pool pool(fee, tickSpacing, maxLiquidityPerTick);

    std::cerr << "Done." << std::endl;
    // std::cerr << "sizeofSlice = " << sizeof(V3Event) << std::endl;

    std::vector<V3Event> data;
    V3Event even; bool eof = false;

    while(!eof) {
        auto [even, eof] = v3::tempReadEventsFile(fin);
        data.push_back(even);
        if(data.size() > 3728270) break;
    }
    std::cerr << "Total events = " << data.size() << std::endl;

    int preProcess = 3628270;
    for(int i = 0; i < preProcess; i++) {
        pool.processEvent(data[i]);
    }

    double Timer = clock();
    for(int i = preProcess; i < data.size(); i++) {
        pool.processEvent(data[i]);
    }
    Timer = (clock() - Timer) / CLOCKS_PER_SEC * 1000 * 1000 * 1000;
    Timer /= (data.size() - preProcess);

    std::cerr << "mean of process event time used = " << Timer << " ns\n";

    return 0;
}