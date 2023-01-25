#include "pool.h"

uint256 dis(uint256 a, uint256 b) {
    if (a < b) std::swap(a, b);
    return a - b;
}

long long getTimeNs() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    return ts.tv_sec*1000000000+ts.tv_nsec;
}

int main() {
    Pool pool(
        "0x1f98431c8ad98523631ae4a59f267346ea31f984",
        "0x2791bca1f2de4661ed88a30c99a7a9449aa84174",
        "0x7ceb23fd6bc0add59e62ac25578270cff1b9f619",
        3000,
        60,
        uint128("11505743598341114571880798222544994")
    );
    std::ios::sync_with_stdio(false);
    freopen("pool_events_test", "r", stdin);
    uint256 ruamount0, ruamount1;
    int256 ramount0, ramount1;
    std::string met, price, sender, amount, amount0, amount1, liquidity;
    int cnt[4] = {0};
    long long timeCnt[4] = {0};
    int tick, tickLower, tickUpper, zeroToOne, t = 0;
    while (std::cin >> met) {
        Pool back = pool;
        std::cin >> sender; msg.sender.FromString(sender);
        // if (t == 338) break;
        // std::cout << ++t << " " << met << std::endl;
        // cnt[met]++;
        long long start = getTimeNs();
        if (met == "initialize") {
            cnt[0]++;
            std::cin >> price >> tick;
            // std::cout << price << " " << tick << std::endl;
            int r = pool.initialize(price);
            assert(r == tick);
            timeCnt[0] += getTimeNs() - start;
        } else if (met == "mint") {
            cnt[1]++;
            std::cin >> tickLower >> tickUpper >> liquidity >> amount0 >> amount1;
            std::tie(ruamount0, ruamount1) = pool.mint(sender, tickLower, tickUpper, liquidity, "");
            timeCnt[1] += getTimeNs() - start;
            // std::cout << ruamount0 << " " << ruamount1 << std::endl;
            // std::cout << amount0 << " " << amount1 << std::endl;
            assert(ruamount0 == amount0 && ruamount1 == amount1);
            assert(dis(ruamount0, amount0) <= 100 && dis(ruamount1, amount1) < 100);
        } else if (met == "swap") {
            cnt[2]++;
            std::cin >> zeroToOne >> amount >> price >> amount0 >> amount1 >> liquidity >> tick;
            // std::cout << zeroToOne << " " << amount << " " << price << std::endl;
            std::string _price = zeroToOne ? "4295128740" : "1461446703485210103287273052203988822378723970341";
            bool suc = false;
            for (int i = 0; i < 3; ++i) {
                if (i) pool = back;
                // std::cout << "================= Swap attempt " << (i + 1) << "==================\n";
                if (i == 0) std::tie(ramount0, ramount1) = pool.swap(sender, zeroToOne, amount, _price, "");
                else if (i == 1) std::tie(ramount0, ramount1) = pool.swap(sender, zeroToOne, zeroToOne ? amount1 : amount0, _price, "");
                else if (i == 2) std::tie(ramount0, ramount1) = pool.swap(sender, zeroToOne, amount + "0", price, "");
                // std::cout << "amount0: " << amount0 << " " << ramount0 << std::endl;
                // std::cout << "amount1: " << amount1 << " " << ramount1 << std::endl;
                // assert(ramount0 == amount0 && ramount1 == amount1);
                // std::cout << "liquidity: " << pool.liquidity << " " << liquidity << std::endl;
                // std::cout << "tick: " << pool.slot0.tick << " " << tick << std::endl;
                // std::cout << "price: " << pool.slot0.sqrtPriceX96 << " " << price << std::endl;
                // assert(pool.liquidity == liquidity && pool.slot0.tick == tick);
                // assert(pool.slot0.sqrtPriceX96 == price);
                // std::cout << "============== Swap attempt " << (i + 1) << " END ================\n";
                if (ramount0 == amount0 && ramount1 == amount1
                    && pool.liquidity == liquidity && pool.slot0.tick == tick
                    && pool.slot0.sqrtPriceX96 == price) {
                    suc = true;
                    break;
                }
            }
            assert(suc);
            timeCnt[2] += getTimeNs() - start;
        } else if (met == "burn") {
            cnt[3]++;
            std::cin >> tickLower >> tickUpper >> amount >> amount0 >> amount1;
            std::tie(ruamount0, ruamount1) = pool.burn(tickLower, tickUpper, amount);
            // std::cout << ruamount0 << " " << ruamount1 << std::endl;
            // std::cout << amount0 << " " << amount1 << std::endl;
            assert(ruamount0 == amount0 && ruamount1 == amount1);
            assert(dis(ruamount0, amount0) <= 100 && dis(ruamount1, amount1) < 100);
            timeCnt[3] += getTimeNs() - start;
        } else if (met == "collect") {
            // std::cin >> tickLower >> tickUpper >> amount0 >> amount1;
            // std::tie(ruamount0, ruamount1) = pool.collect("", tickLower, tickUpper, amount0, amount1);
            // std::cout << ruamount0 << " " << ruamount1 << std::endl;
            // std::cout << amount0 << " " << amount1 << std::endl;
            // assert(ruamount0 == amount0 && ruamount1 == amount1);
        }
        // pool.slot0.print();
        // std::cout << "sqrtPriceX96: ";
        // pool.slot0.sqrtPriceX96.PrintTable(std::cout);
        // std::cout << std::endl;
        // std::cout << "liquidity: " << pool.liquidity << std::endl;
    }
    for (int i = 0; i < 4; ++i) {
        std::cout << i << " " << cnt[i] << " " << timeCnt[i] << std::endl;
    }
    // std::tie(uamount0, uamount1) = pool.mint(
    //     "0xc36442b4a4522e871399cd717abdd847ab11fe88",
    //     193320,
    //     193620,
    //     "176527367721",
    //     ""
    // );
    // assert(uamount0 == 0 && uamount1 == "42055547280282");
    // std::cout << "sqrtPriceX96: " << pool.slot0.sqrtPriceX96 << std::endl;
    // std::cout << "liquidity: " << pool.liquidity << std::endl;

    // std::tie(uamount0, uamount1) = pool.mint(
    //     "0xc36442b4a4522e871399cd717abdd847ab11fe88",
    //     193320,
    //     193860,
    //     "944144821117635",
    //     ""
    // );
    // std::cout << uamount0 << " " << uamount1 << std::endl;
    // assert(uamount0 == "615723603" && uamount1 == "247499998952080517");
    // std::cout << "sqrtPriceX96: " << pool.slot0.sqrtPriceX96 << std::endl;
    // std::cout << "liquidity: " << pool.liquidity << std::endl;

    // std::tie(uamount0, uamount1) = pool.mint(
    //     "0xc36442b4a4522e871399cd717abdd847ab11fe88",
    //     184200,
    //     207240,
    //     "650015491760",
    //     ""
    // );
    // std::cout << uamount0 << " " << uamount1 << std::endl;
    // assert(uamount0 == "20000000" && uamount1 == "3922623145361175");
    // std::cout << "sqrtPriceX96: " << pool.slot0.sqrtPriceX96 << std::endl;
    // std::cout << "liquidity: " << pool.liquidity << std::endl;

    // std::tie(uamount0, uamount1) = pool.mint(
    //     "0xc36442b4a4522e871399cd717abdd847ab11fe88",
    //     168120,
    //     207240,
    //     "321757635921",
    //     ""
    // );
    // std::cout << uamount0 << " " << uamount1 << std::endl;
    // assert(uamount0 == "9899999" && uamount1 == "3717814976258994");
    // std::cout << "sqrtPriceX96: " << pool.slot0.sqrtPriceX96 << std::endl;
    // std::cout << "liquidity: " << pool.liquidity << std::endl;

    // int256 amount0, amount1;
    // std::tie(amount0, amount1) = pool.swap(
    //     "0xe866ece4bbd0ac75577225ee2c464ef16dc8b1f3",
    //     true,
    //     "1000000",
    //     "1267868630852020208767707712718408",
    //     ""
    // );
    
    // assert(amount0 == "1000000");
    // assert(amount1 == int256("-256078805821986"));

    // std::tie(amount0, amount1) = pool.swap(
    //     "0x2ec255797fef7669fa243509b7a599121148ffba",
    //     true,
    //     "85000000",
    //     "1267918992828562240687553738070979",
    //     ""
    // );
    
    // std::cout << amount0 << " " << amount1 << std::endl;
    // assert(amount0 == "85000000");
    // assert(amount1 == int256("-21735096630391110"));

    // std::tie(amount0, amount1) = pool.swap(
    //     "0x2ec255797fef7669fa243509b7a599121148ffba",
    //     true,
    //     "10822795",
    //     "1267687408174265640077661028005782",
    //     ""
    // );
    
    // std::cout << amount0 << " " << amount1 << std::endl;
    // assert(amount0 == "10822795");
    // assert(amount1 == int256("-2762988356206610"));
}