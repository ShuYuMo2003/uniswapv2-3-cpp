#include "../include/pool.h"
#include <cstdlib>
#include <cassert>
#include <iomanip>
#include <cstdlib>

uint256 dis(uint256 a, uint256 b) {
    if (a < b) std::swap(a, b);
    return a - b;
}

long long getTimeNs() {
    return 1LL * clock() * 1000 * 1000 * 1000 / CLOCKS_PER_SEC; // ns
}

const int repeatTimes = 1000;
double MAX_DIFF = -1;
double TOT_DIFF = 0;
int    TOT_CNT  = 0;
const double MAX_DEVIATION = 0.001;



std::pair<uint256, uint256> swapWithCheck(
    Pool<false> * pool,
    Pool<true> * PoolFloat,
    address a,
    bool zeroToOne,
    int256 amountSpecified,
    uint160 sqrtPriceLimitX96,
    bytes32 e,
    long long & timer)
{ // recover the comments below to test the switch of the effects on pool state.
    // pool.save("POOL_STATE_BEFORE");
    // timer = getTimeNs();
    // auto ret0 = pool.swap(a, b, c, d, e, false);
    // timer = getTimeNs() - timer;
    // assert(timer != 0);
    // pool.save("POOL_STATE_AFTER");

    // assert(system("diff POOL_STATE_BEFORE POOL_STATE_AFTER") == 0);
    // std::cerr << "================================================= START ============================================" << std::endl;

    // std::cerr << "====== 1 FIRST SWAP START =======" << std::endl;

    // std::cerr << "====== 1 FIRST SWAP END  =======" << std::endl << std::endl;



    // std::cerr << "====== 2 FIRST SWAP START =======" << std::endl;

    // std::cerr << "====== 2 FIRST SWAP END  =======" << std::endl;


    // std::cout << "==========================" << std::endl;
    // std::cout << std::setiosflags(std::ios::fixed) << std::setprecision(0);
    // std::cout << ret0.first << " " << ret0.second << std::endl;
    // std::cout << ret1.first << " " << ret1.second << std:: endl;
    // cls && g++ pool_test.cpp -o a -Wall -std=c++17 -O3 && .\a
    // std::cerr << "================================================= END ============================================" << std::endl << std::endl;

// calc time:
    // std::pair<double, double> ret_;
    // timer = getTimeNs();
    // for(int i = 0; i < repeatTimes; i++)
    //     ret_ = pool.swap_effectless(a, zeroToOne, amountSpecified, d, e);
    // timer = double(getTimeNs() - timer) / repeatTimes;


// not calc time:
    // std::cerr << "====================== FloatT =====================" << std::endl;
    auto ret0 = swap(PoolFloat, zeroToOne, amountSpecified.ToDouble(), sqrtPriceLimitX96.X96ToDouble(), true);
    // std::cerr << "====================== BigInt =====================" << std::endl;
    auto ret1 = swap(pool, zeroToOne, amountSpecified, sqrtPriceLimitX96, true);


    double diffe;
    if (zeroToOne^(amountSpecified > 0)) {
        diffe = fabs(  (ret1.second.ToDouble() - ret0.second) / std::max(fabs(ret0.second), fabs(ret1.second.ToDouble()))  );
    } else {
        diffe = fabs(  (ret1.first.ToDouble() - ret0.first) / std::max(fabs(ret0.first), fabs(ret1.first.ToDouble()))  );
    }

    if(diffe < MAX_DEVIATION || (ret1.first > -1000 && ret1.first < 1000) || (ret1.second > -1000 && ret1.second < 1000)) {
        if ((ret1.first > 1000 || ret1.first < -1000) && (ret1.second > 1000 || ret1.second < -1000)) {
            // if (diffe > 0.001) std::cout << ret1.first << " " << ret1.second << " " << diffe << std::endl;
            MAX_DIFF = std::max(MAX_DIFF, diffe);
            TOT_DIFF += diffe; TOT_CNT ++;
        }
    } else {
        static char buffer[1000];
        sprintf(buffer, "\n\n================================================= FAIL ============================================\n"
                        "%.30lf %.30lf\n%.30lf %.30lf\n",
                ret0.first, ret0.second, ret1.first.ToDouble(), ret1.second.ToDouble());
        std::cerr << buffer << std::endl;
        exit(0);
    }

    // std::cerr << "=========================== End of Swap ===========================" << std::endl;
    // std::cerr << "float "; PoolFloat->slot0.print();
    // std::cerr<< "==========================================="  << std::endl;
    // std::cerr << "bigin "; pool->slot0.print();


    return ret1;
}

double comparison(double x, double y) {
    if(fabs(x - y) < EPS) return 0;
    return fabs(x - y) / std::max(x, y);
}

int main(int argc, char *argv[]) {
    std::ifstream slot0saver("pool_slot0_state");
    std::ofstream hideinfo("hideinfo");
    std::cout << std::setiosflags(std::ios::fixed) << std::setprecision(20);
    std::cerr << std::setiosflags(std::ios::fixed) << std::setprecision(7);
    std::cerr << "Initializing tick price" << std::endl;
    initializeTicksPrice();
    std::cerr << "Tick price initialized" << std::endl;
    // std::ios::sync_with_stdio(false);
    freopen("pool_events_test_", "r", stdin);
    int fee, tickSpacing;
    uint256 maxLiquidityPerTick;
    std::cin >> fee >> tickSpacing >> maxLiquidityPerTick;

    Pool<false> *pool = (Pool<false> *)malloc(1024 * 1024); // 1 MB.
    Pool<false> *back = (Pool<false> *)malloc(1024 * 1024); // 1 MB.

    Pool<true> *poolFloat = (Pool<true> *)malloc(1024 * 1024);
    Pool<true> *backFloat = (Pool<true> *)malloc(1024 * 1024);



    // FILE * fptr = fopen("pool_temp_state", "r");
    // fread(pool, 1, 1024 * 1024, fptr);
    // fclose(fptr);
    // std::cerr << "fee = " << pool->fee << std::endl;


    Pool<false> temppool(fee, tickSpacing, maxLiquidityPerTick);
    CopyPool(&temppool, pool);


    uint256 ruamount0, ruamount1;
    int256 ramount0, ramount1;
    std::string met, price, sender, amount, amount0, amount1, liquidity;
    FloatType ruamount0_fuck, ruamount1_fuck;
    int cnt[4] = {0};
    long long timeCnt[4] = {0};
    int tick, tickLower, tickUpper, zeroToOne, t = 0, blockNum;

    GenerateFloatPool(pool, poolFloat);

    int lastBlock = 0;
    while (std::cin >> met) {
        std::cin >> sender;
        ++t;

        if(t % 20000 == 0) std::cerr << "to handle " << t << std::endl;
        if (met == "initialize") std::cin >> price >> tick;
        else if (met == "mint") std::cin >> tickLower >> tickUpper >> liquidity >> amount0 >> amount1;
        else if (met == "swap") std::cin >> zeroToOne >> amount >> price >> amount0 >> amount1 >> liquidity >> tick;
        else if (met == "burn") std::cin >> tickLower >> tickUpper >> amount >> amount0 >> amount1;
        std::cin >> blockNum;

        // if (t <= 446538) continue;

        if(lastBlock != blockNum) {
            GenerateFloatPool(pool, poolFloat);
            lastBlock = blockNum;
        }
        CopyPool(pool, back);
        CopyPool(poolFloat, backFloat);

        // std::cerr << "\n\n======================== Got contract = " << met << " No." << (t) << " =========================== "<< std::endl;
        if (met == "initialize") {
            cnt[0]++;
            long long start = getTimeNs();
            int r      = initialize(pool,      price);
            int r_fuck = initialize(poolFloat, uint160(price).X96ToDouble());
            // std::cout << price << " " << r << " " << tick << std::endl;
            assert(r_fuck == r && r == tick);
            timeCnt[0] += getTimeNs() - start;
        } else if (met == "mint") {
            cnt[1]++;
            long long start = getTimeNs();
            // std::cerr << "==== BIGINT Pool ====" << std::endl;
            std::tie(ruamount0, ruamount1)           = mint(pool,      tickLower, tickUpper, liquidity, "");
            // std::cerr << "==== FLOAT  Pool ====" << std::endl;
            std::tie(ruamount0_fuck, ruamount1_fuck) = mint(poolFloat,  tickLower, tickUpper, uint128(liquidity).ToDouble(), "");
            timeCnt[1] += getTimeNs() - start;
            // std::cout << ruamount0 << " " << ruamount1 << std::endl;
            // std::cout << amount0 << " " << amount1 << std::endl;
            assert(ruamount0 == amount0 && ruamount1 == amount1);
            // std::cerr << "MINT RET: " << ruamount0.ToDouble() << " " << ruamount0_fuck << std::endl;
            // std::cerr << "MINT RET: " << ruamount1.ToDouble() << " " << ruamount1_fuck << std::endl;
            assert(comparison(ruamount0.ToDouble(), ruamount0_fuck) < MAX_DEVIATION);
            assert(comparison(ruamount1.ToDouble(), ruamount1_fuck) < MAX_DEVIATION);
            // assert(dis(ruamount0, amount0) <= 100 && dis(ruamount1, amount1) < 100);
        } else if (met == "swap") {
            cnt[2]++;
            // std::cout << zeroToOne << " " << amount << " " << price << std::endl;
            std::string _price = zeroToOne ? "4295128740" : "1461446703485210103287273052203988822378723970341";
            bool suc = false;
            long long timer = 0;
            for (int i = 0; i < 3; ++i) {
                if (i) {
                    CopyPool(back, pool);
                    CopyPool(backFloat, poolFloat);
                }
                // std::cout << "\n================= Swap attempt " << (i + 1) << "==================\n";
                if (i == 0) std::tie(ramount0, ramount1) = swapWithCheck(pool, poolFloat, sender, zeroToOne, amount, _price, "", timer);
                else if (i == 1) {
                    std::string k = zeroToOne ? amount1 : amount0;
                    if (k == "0") continue;
                    std::tie(ramount0, ramount1) = swapWithCheck(pool, poolFloat, sender, zeroToOne, k, _price, "", timer);
                }
                else if (i == 2) std::tie(ramount0, ramount1) = swapWithCheck(pool, poolFloat, sender, zeroToOne, amount + "0", price, "", timer);
                // std::cout << "amount0: " << ramount0 << " " << amount0 << std::endl;
                // std::cout << "amount1: " << ramount1 << " " << amount1 << std::endl;
                // std::cout << "liquidity: " << pool->liquidity << " " << liquidity << std::endl;
                // std::cout << "tick: " << pool->slot0.tick << " " << tick << std::endl;
                // std::cout << "price: " << pool->slot0.sqrtPriceX96 << " " << price << std::endl;
                // std::cout << "============== Swap attempt " << (i + 1) << " END ================\n";
                if (ramount0 == amount0 && ramount1 == amount1
                    && pool->liquidity == liquidity && pool->slot0.tick == tick
                    && pool->slot0.sqrtPriceX96 == price) {
                    suc = true;
                    break;
                }
            }
            assert(suc);
            timeCnt[2] += timer;
        } else if (met == "burn") {
            cnt[3]++;
            long long start = getTimeNs();
            // std::cerr << "============== BIGINT ================" << std::endl;
            std::tie(ruamount0, ruamount1)           = burn(pool,      tickLower, tickUpper, amount);
            // std::cerr << "============== FLOAT  ================" << std::endl;
            std::tie(ruamount0_fuck, ruamount1_fuck) = burn(poolFloat, tickLower, tickUpper, uint128(amount).ToDouble());
            // std::cerr << "============== ENDDD ================" << std::endl;
            // std::cout << tickLower << " " << tickUpper << " " << amount << std::endl;
            // std::cout << ruamount0 << " " << ruamount1 << std::endl;
            // std::cout << amount0 << " " << amount1 << std::endl;

            // std::cerr << "BURN RET: " << ruamount0.ToDouble() << " " << ruamount0_fuck << std::endl;
            // std::cerr << "BURN RET: " << ruamount1.ToDouble() << " " << ruamount1_fuck << std::endl;
            assert(ruamount0 == amount0 && ruamount1 == amount1);
            assert(dis(ruamount0, amount0) <= 100 && dis(ruamount1, amount1) < 100);
            assert(comparison(ruamount0.ToDouble(), ruamount0_fuck) < MAX_DEVIATION);
            assert(comparison(ruamount1.ToDouble(), ruamount1_fuck) < MAX_DEVIATION);
            timeCnt[3] += getTimeNs() - start;
        } else if (met == "collect") {
            std::cin >> tickLower >> tickUpper >> amount0 >> amount1;
            // std::tie(ruamount0, ruamount1) = pool.collect("", tickLower, tickUpper, amount0, amount1);
            // std::cout << ruamount0 << " " << ruamount1 << std::endl;
            // std::cout << amount0 << " " << amount1 << std::endl;
            // assert(ruamount0 == amount0 && ruamount1 == amount1);
        }
        // std::string slprice; int sltick, tickcnt;
        // slot0saver >> slprice >> sltick >> tickcnt;

        // if(uint160(slprice) == pool->slot0.sqrtPriceX96 && sltick == pool->slot0.tick
        //     && tickcnt == pool->ticks.length) {
        //     // std::cerr << "QAQ" << std::endl;
        // } else {
        //     std::cerr << slprice << " " << sltick << std::endl;
        //     std::cerr << pool->slot0.sqrtPriceX96 << " " << pool->slot0.tick << std::endl;
        //     assert(false);
        // }
        // // // assert(comparison(pool.slot0.sqrtPriceX96.X96ToDouble(), PoolFloat.slot0.sqrtPriceX96) < MAX_DEVIATION);
        // // // assert(pool.slot0.tick == PoolFloat.slot0.tick);
        // std::cerr << "+++ initialized ticks: " << std::endl;
        // for(int i = 0; i < pool->ticks.length; i++) {
        //     std::cerr << *((&pool->ticks.temp + 1) + i) << std::endl;
        // }
        // std::cerr << "+++" << std::endl;
        // hideinfo << t << std::endl;
        // if(t == 446538) {
        //     FILE * fptr = fopen("pool_temp_state", "w");
        //     fwrite(pool, 1, 1024 * 1024, fptr);
        //     fclose(fptr);
        // }
    }
    std::cout << "\n\n" << std::endl;
    for (int i = 0; i < 4; ++i) {
        std::cout << i << " " << cnt[i] << " " << timeCnt[i] << std::endl;
    }
    std::cout << "============= Timer ============" << std::endl;
    std::cout << "init: \t" << ((long double) (timeCnt[0]) / cnt[0])  << " ns/opt" << std::endl;
    std::cout << "mint: \t" << ((long double) (timeCnt[1]) / cnt[1])  << " ns/opt" << std::endl;
    std::cout << "swap: \t" << ((long double) (timeCnt[2]) / cnt[2])  << " ns/opt" << std::endl;
    std::cout << "burn: \t" << ((long double) (timeCnt[3]) / cnt[3] ) << " ns/opt" << std::endl;

    std::cout << "\n============ mistakes ===========" << std::endl;
    std::cout << "SWAP without effect \taverage mistake = \t" << (TOT_DIFF / TOT_CNT) << std::endl;
    std::cout << "\t\t\tmaximum mistake = \t" << MAX_DIFF << std::endl;
    // std::cerr << "cache rate = " << pool.tickBitmap.cacheRate() << " " << pool.tickBitmap.cacheMiss << " " << pool.tickBitmap.cacheTotal<< std::endl;

    SavePool(pool, "pool_state");
    free(pool);
    free(back);
    free(poolFloat);
    free(backFloat);
    return 0;
}