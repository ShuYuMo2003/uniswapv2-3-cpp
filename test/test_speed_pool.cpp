#include <iostream>
#include <cstdio>
#include <ctime>
#include "../include/types.h"
#include "../include/pool.h"

using namespace std;

int UNI_DATA_SIZE = -1;
const int TEST_TIME = 1e7;

int TOT_CNT = 0;
double MAX_DIFF = -1, TOT_DIFF = 0;


std::pair<double, double> swap_handle(Pool<true> * pool, bool zeroToOne, double amountIn) {
    static double SQPRL = uint160("4295128740").X96ToDouble();
    static double SQPRR = uint160("1461446703485210103287273052203988822378723970341").X96ToDouble();
    return swap(pool, zeroToOne, amountIn, zeroToOne ? SQPRL : SQPRR, false);
}

std::pair<int256, int256> raw_swap_handle(Pool<false> * pool, bool zeroToOne, int256 amountIn) {
    static uint160 SQPRL = uint160("4295128740");
    static uint160 SQPRR = uint160("1461446703485210103287273052203988822378723970341");
    return swap(pool, zeroToOne, amountIn, zeroToOne ? SQPRL : SQPRR, false);
}


struct testcase{
    bool zeroToOne;
    int256 raw_amount;
    double amount;
    std::pair<double, double> result;
};
vector<testcase> tc;
vector<pair<double, double>> result;

void generate(Pool<false> * pool) {
    for(uint160 amount = 10; amount < uint160("94033269757636"); (amount *= 16) /= 10) {
        testcase now;
        now.zeroToOne  =  1;
        now.raw_amount =  amount;
        now.amount     =  now.raw_amount.ToDouble();
        std::pair<int256, int256> ret = raw_swap_handle(pool, now.zeroToOne, now.raw_amount);
        now.result     = make_pair(ret.first.ToDouble(), ret.second.ToDouble());
        tc.push_back(now);
        // cout << amount << endl;
    }
    for(uint160 amount = 10; amount < uint160("65308223357628934551218"); (amount *= 16) /= 10) {
        testcase now;
        now.zeroToOne  =  0;
        now.raw_amount =  amount;
        now.amount     =  now.raw_amount.ToDouble();
        std::pair<int256, int256> ret = raw_swap_handle(pool, now.zeroToOne, now.raw_amount);
        now.result     = make_pair(ret.first.ToDouble(), ret.second.ToDouble());
        tc.push_back(now);
        // cout << amount << endl;
    }
    UNI_DATA_SIZE = tc.size();
    result.resize(UNI_DATA_SIZE);
    random_shuffle(tc.begin(), tc.end());
}

/*
address: 0x88e6A0c2dDD26FEEb64F039a2c41296FcB3f5640
WETH    (0xc02aaa39b223fe8d0a0e5c4f27ead9083c756cc2) (1): 65308223357628934551218
USD Coin(0xa0b86991c6218b36c1d19d4a2e9eb0ce3606eb48) (0): 94033269757636
*/

int main(){
    cerr << "Initializing tick." << endl;
    initializeTicksPrice();
    cerr << "Generating data." << endl;

    Pool<false> pool("pool_state");
    generate(&pool);
    cerr << "done generated data cnt = " << UNI_DATA_SIZE << endl;
    Pool<true> pool_float;
    GenerateFloatPool(&pool, &pool_float);

    cerr << "run" << endl;
    double timer = clock(); int uniq_id = 0;
    for(int i = 0; i < TEST_TIME; i++, uniq_id++) {
        if(uniq_id == UNI_DATA_SIZE) uniq_id = 0;
        result[uniq_id] = swap_handle(&pool_float, tc[uniq_id].zeroToOne, tc[uniq_id].amount);
    }
    timer = (clock() - timer) / CLOCKS_PER_SEC * 1000 * 1000 * 1000;
    timer /= TEST_TIME;
    cerr << "run done" << endl;

    for(int i = 0; i < UNI_DATA_SIZE; i++) {
        pair<double, double> ret1 = result[i], ret0 = tc[i].result;
        // cout << ret0.first << " " << ret1.first << endl;
        // cout << ret0.second << " " << ret1.second << endl;
        bool zeroToOne = tc[i].zeroToOne;
        double amountSpecified = tc[i].amount;
        double diffe;

        diffe = max(fabs(  (ret1.second - ret0.second) / std::max(fabs(ret0.second), fabs(ret1.second))  ),
                fabs(  (ret1.first - ret0.first) / std::max(fabs(ret0.first), fabs(ret1.first))  ));

        MAX_DIFF = std::max(MAX_DIFF, diffe);
        TOT_DIFF += diffe; TOT_CNT ++;
        if(diffe < 0.000001 || (ret1.first > -100000 && ret1.first < 100000) || (ret1.second > -100000 && ret1.second < 100000)) ; else {
            static char buffer[1000];
            sprintf(buffer, "\n\n================================================= FAIL ============================================\n"
                            "%.30lf %.30lf\n%.30lf %.30lf\n",
                    ret0.first, ret0.second, ret1.first, ret1.second);
            std::cerr << buffer << std::endl;
        }
    }

    printf("Time used        = %.5lf ns\n", timer);
    printf("Maximum mistake  = %.30lf\n", MAX_DIFF);
    printf("Average mistake  = %.30f\n", TOT_DIFF / TOT_CNT);
    return 0;
}
