#include "../include/pool.h"
#include <cstdlib>
#include <cassert>
#include <iomanip>

uint256 dis(uint256 a, uint256 b) {
    if (a < b) std::swap(a, b);
    return a - b;
}

long long getTimeNs() {
    return 1LL * clock() * 1000 * 1000 * 1000 / CLOCKS_PER_SEC; // ns
}

const int repeatTimes = 100000;
double MAX_DIFF = -1;
double TOT_DIFF = 0;
int    TOT_CNT  = 0;


std::pair<uint256, uint256> swapWithCheck(
    Pool & pool,
    address a,
    bool b,
    int256 c,
    uint160 d,
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
    auto ret1 = pool.swap(a, b, c, d, e);
    // std::cerr << "====== 2 FIRST SWAP END  =======" << std::endl;


    // std::cout << "==========================" << std::endl;
    // std::cout << std::setiosflags(std::ios::fixed) << std::setprecision(0);
    // std::cout << ret0.first << " " << ret0.second << std::endl;
    // std::cout << ret1.first << " " << ret1.second << std:: endl;
    // cls && g++ pool_test.cpp -o a -Wall -std=c++17 -O3 && .\a
    // std::cerr << "================================================= END ============================================" << std::endl << std::endl;
    std::pair<double, double> ret0;
    timer = getTimeNs();
    for(int i = 0; i < repeatTimes; i++)
        ret0 = pool.swap_effectless(a, b, c, d, e);
    assert(timer != getTimeNs());
    timer = double(getTimeNs() - timer) / repeatTimes;


    double diffe = fabs(  (ret1.second.ToDouble() - ret0.second) / std::max(ret0.second, ret1.second.ToDouble())  );

    if(diffe < 0.02) ; else {
        static char buffer[1000];
        sprintf(buffer, "\n\n================================================= FAIL ============================================\n"
                        "%.30lf %.30lf\n%.30lf %.30lf\n",
                ret0.first, ret0.second, ret1.first.ToDouble(), ret1.second.ToDouble());
        std::cerr << buffer << std::endl;
    }

    MAX_DIFF = std::max(MAX_DIFF, diffe);
    TOT_DIFF += diffe; TOT_CNT ++;


    return ret1;
}

int main(int argc, char *argv[]) {
    std::cout << std::setiosflags(std::ios::fixed) << std::setprecision(0);
    std::cerr << "Initializing tick price" << std::endl;
    initializeTicksPrice();
    std::cerr << "done" << std::endl;
    std::ios::sync_with_stdio(false);
    freopen("pool_events_test", "r", stdin);
    int fee, tickSpacing;
    uint256 maxLiquidityPerTick;
    std::cin >> fee >> tickSpacing >> maxLiquidityPerTick;
    Pool pool;
    int stBlock = 0;
    if (argc > 1) {
        stBlock = atoi(argv[1]);
        std::ifstream fin(argv[2]);
        fin >> pool;
        fin.close();
    } else {
        pool = Pool(fee, tickSpacing, maxLiquidityPerTick);
    }
    // pool.save("tmp0");
    uint256 ruamount0, ruamount1;
    int256 ramount0, ramount1;
    std::string met, price, sender, amount, amount0, amount1, liquidity;
    int cnt[4] = {0};
    long long timeCnt[4] = {0};
    int tick, tickLower, tickUpper, zeroToOne, t = 0, blockNum;
    while (std::cin >> met) {

        Pool back = pool;
        // Pool pool("tmp" + std::to_string(t)), back = pool;
        // pool.save("tmpread" + std::to_string(t));
        std::cin >> sender; msg.sender.FromString(sender);
        ++t;
        // std::cerr << "Got contract = " << met << " No." << (t) << std::endl;
        if(t % 300 == 0) std::cerr << "\rto handle " << t;
        if (met == "initialize") std::cin >> price >> tick;
        else if (met == "mint") std::cin >> tickLower >> tickUpper >> liquidity >> amount0 >> amount1;
        else if (met == "swap") std::cin >> zeroToOne >> amount >> price >> amount0 >> amount1 >> liquidity >> tick;
        else if (met == "burn") std::cin >> tickLower >> tickUpper >> amount >> amount0 >> amount1;
        std::cin >> blockNum;
        // int lim = 1268728;
        if (blockNum <= stBlock) continue;
        // else if (t == lim) { pool = Pool("tmp" + std::to_string(t)); continue; }
        // std::cout << t << " " << met << std::endl;
        if (met == "initialize") {
            cnt[0]++;
            long long start = getTimeNs();
            int r = pool.initialize(price);
            // std::cout << price << " " << r << " " << tick << std::endl;
            assert(r == tick);
            timeCnt[0] += getTimeNs() - start;
        } else if (met == "mint") {
            cnt[1]++;
            long long start = getTimeNs();
            std::tie(ruamount0, ruamount1) = pool.mint(sender, tickLower, tickUpper, liquidity, "");
            timeCnt[1] += getTimeNs() - start;
            // std::cout << ruamount0 << " " << ruamount1 << std::endl;
            // std::cout << amount0 << " " << amount1 << std::endl;
            assert(ruamount0 == amount0 && ruamount1 == amount1);
            // assert(dis(ruamount0, amount0) <= 100 && dis(ruamount1, amount1) < 100);
        } else if (met == "swap") {
            cnt[2]++;
            // std::cout << zeroToOne << " " << amount << " " << price << std::endl;
            std::string _price = zeroToOne ? "4295128740" : "1461446703485210103287273052203988822378723970341";
            bool suc = false;
            long long timer = 0;
            for (int i = 0; i < 3; ++i) {
                if (i) pool = back;
                // std::cout << "================= Swap attempt " << (i + 1) << "==================\n";
                if (i == 0) std::tie(ramount0, ramount1) = swapWithCheck(pool, sender, zeroToOne, amount, _price, "", timer);
                else if (i == 1) {
                    std::string k = zeroToOne ? amount1 : amount0;
                    if (k == "0") continue;
                    std::tie(ramount0, ramount1) = swapWithCheck(pool, sender, zeroToOne, k, _price, "", timer);
                }
                else if (i == 2) std::tie(ramount0, ramount1) = swapWithCheck(pool, sender, zeroToOne, amount + "0", price, "", timer);
                // std::cout << "amount0: " << ramount0 << " " << amount0 << std::endl;
                // std::cout << "amount1: " << ramount1 << " " << amount1 << std::endl;
                // // assert(ramount0 == amount0 && ramount1 == amount1);
                // std::cout << "liquidity: " << pool.liquidity << " " << liquidity << std::endl;
                // std::cout << "tick: " << pool.slot0.tick << " " << tick << std::endl;
                // std::cout << "price: " << pool.slot0.sqrtPriceX96 << " " << price << std::endl;
                // // assert(pool.liquidity == liquidity && pool.slot0.tick == tick);
                // // assert(pool.slot0.sqrtPriceX96 == price);
                // std::cout << "============== Swap attempt " << (i + 1) << " END ================\n";
                if (ramount0 == amount0 && ramount1 == amount1
                    && pool.liquidity == liquidity && pool.slot0.tick == tick
                    && pool.slot0.sqrtPriceX96 == price) {
                    suc = true;
                    break;
                }
            }
            assert(suc);
            timeCnt[2] += timer;
        } else if (met == "burn") {
            cnt[3]++;
            long long start = getTimeNs();
            std::tie(ruamount0, ruamount1) = pool.burn(tickLower, tickUpper, amount);
            // std::cout << tickLower << " " << tickUpper << " " << amount << std::endl;
            // std::cout << ruamount0 << " " << ruamount1 << std::endl;
            // std::cout << amount0 << " " << amount1 << std::endl;
            assert(ruamount0 == amount0 && ruamount1 == amount1);
            assert(dis(ruamount0, amount0) <= 100 && dis(ruamount1, amount1) < 100);
            timeCnt[3] += getTimeNs() - start;
        } else if (met == "collect") {
            std::cin >> tickLower >> tickUpper >> amount0 >> amount1;
            // std::tie(ruamount0, ruamount1) = pool.collect("", tickLower, tickUpper, amount0, amount1);
            // std::cout << ruamount0 << " " << ruamount1 << std::endl;
            // std::cout << amount0 << " " << amount1 << std::endl;
            // assert(ruamount0 == amount0 && ruamount1 == amount1);
        }
        // std::cout << pool << std::endl;
        // pool.save("tmp" + std::to_string(t));
        // if (t > 10) std::system(("rm tmp" + std::to_string(t - 10)).c_str());
        // pool.slot0.print();
        // std::cout << "sqrtPriceX96: " <<;
        // pool.slot0.sqrtPriceX96.PrintTable(std::cout);
        // std::cout << std::endl;
        // std::cout << "liquidity: " << pool.liquidity << std::endl;
    }
    std::cout << "\n\n" << std::endl;
    for (int i = 0; i < 4; ++i) {
        std::cout << i << " " << cnt[i] << " " << timeCnt[i] << std::endl;
    }
    std::cout << "============= Timer ============" << std::endl;
    std::cout << std::setiosflags(std::ios::fixed) << std::setprecision(6);
    std::cout << "init: \t" << ((long double) (timeCnt[0]) / cnt[0])  << " ns/opt" << std::endl;
    std::cout << "mint: \t" << ((long double) (timeCnt[1]) / cnt[1])  << " ns/opt" << std::endl;
    std::cout << "swap: \t" << ((long double) (timeCnt[2]) / cnt[2])  << " ns/opt" << std::endl;
    std::cout << "burn: \t" << ((long double) (timeCnt[3]) / cnt[3] ) << " ns/opt" << std::endl;

    std::cout << "\n============ mistakes ===========" << std::endl;
    std::cout << "SWAP without effect \taverage mistake = \t" << (TOT_DIFF / TOT_CNT) << std::endl;
    std::cout << "\t\t\tmaximum mistake = \t" << MAX_DIFF << std::endl;
    // std::cerr << "cache rate = " << pool.tickBitmap.cacheRate() << " " << pool.tickBitmap.cacheMiss << " " << pool.tickBitmap.cacheTotal<< std::endl;

    pool.save("pool_state");
}