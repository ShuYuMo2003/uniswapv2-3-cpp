#include "pool.h"
#include <cstdlib>
#include <cassert>

uint256 dis(uint256 a, uint256 b) {
    if (a < b) std::swap(a, b);
    return a - b;
}

long long getTimeNs() {
    return clock();
}

std::pair<uint256, uint256> swapWithCheck(
    Pool & pool,
    address a,
    bool b,
    int256 c,
    uint160 d,
    bytes32 e)
{
    pool.save("POOL_STATE_BEFORE");
    auto ret0 = pool.swap(a, b, c, d, e, false);
    pool.save("POOL_STATE_AFTER");

    assert(system("diff POOL_STATE_BEFORE POOL_STATE_AFTER") == 0);

    auto ret1 = pool.swap(a, b, c, d, e, true);

    assert(ret0 == ret1);
    // std::cerr << "swap " << b << "ok" << std::endl;
    return ret0;
}

int main(int argc, char *argv[]) {
    int Not = 0;
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
        // std::cerr << "Got contract = " << met << " No." << (++Not) << std::endl;
        Pool back = pool;
        // Pool pool("tmp" + std::to_string(t)), back = pool;
        // pool.save("tmpread" + std::to_string(t));
        std::cin >> sender; msg.sender.FromString(sender);
        ++t;
        if (met == "initialize") std::cin >> price >> tick;
        else if (met == "mint") std::cin >> tickLower >> tickUpper >> liquidity >> amount0 >> amount1;
        else if (met == "swap") std::cin >> zeroToOne >> amount >> price >> amount0 >> amount1 >> liquidity >> tick;
        else if (met == "burn") std::cin >> tickLower >> tickUpper >> amount >> amount0 >> amount1;
        std::cin >> blockNum;
        // int lim = 1268728;
        if (blockNum <= stBlock) {
            if (t % 200000 == 0) std::cout << t << " events handled." << std::endl;
            continue;
        }
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
            for (int i = 0; i < 3; ++i) {
                if (i) pool = back;
                // std::cout << "================= Swap attempt " << (i + 1) << "==================\n";
                long long start = getTimeNs();
                if (i == 0) std::tie(ramount0, ramount1) = swapWithCheck(pool, sender, zeroToOne, amount, _price, "");
                else if (i == 1) {
                    std::string k = zeroToOne ? amount1 : amount0;
                    if (k == "0") continue;
                    std::tie(ramount0, ramount1) = swapWithCheck(pool, sender, zeroToOne, k, _price, "");
                }
                else if (i == 2) std::tie(ramount0, ramount1) = swapWithCheck(pool, sender, zeroToOne, amount + "0", price, "");
                timeCnt[2] += getTimeNs() - start;
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
        if (t % 200000 == 0) std::cout << t << " events handled." << std::endl;
    }
    for (int i = 0; i < 4; ++i) {
        std::cout << i << " " << cnt[i] << " " << timeCnt[i] << std::endl;
    }
    pool.save("pool_state");
}