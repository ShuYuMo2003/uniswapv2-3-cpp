#include <iostream>
#include <cstdio>
#include <ctime>
#include "../include/types.h"
#include "../include/pool.h"

using namespace std;

const int UNI_DATA_SIZE = 150;
const int TEST_TIME = 5e5;

int TOT_CNT = 0;
double MAX_DIFF = -1, TOT_DIFF = 0;
pair<double, double> result[UNI_DATA_SIZE];


std::pair<double, double> swap_handle(Pool & pool, bool zeroToOne, double amountIn) {
    static double SQPRL = uint160("4295128740").X96ToDouble();
    static double SQPRR = uint160("1461446703485210103287273052203988822378723970341").X96ToDouble();
    return pool.swap_effectless(zeroToOne, amountIn, zeroToOne ? SQPRL : SQPRR);
}

std::pair<int256, int256> raw_swap_handle(Pool & pool, bool zeroToOne, int256 amountIn) {
    static uint160 SQPRL = uint160("4295128740");
    static uint160 SQPRR = uint160("1461446703485210103287273052203988822378723970341");
    return pool.swap("0x0", zeroToOne, amountIn, zeroToOne ? SQPRL : SQPRR, "", false);
}



struct testcase{
    bool zeroToOne;
    int256 raw_amount;
    double amount;
    std::pair<double, double> result;
} tc[UNI_DATA_SIZE];

void generate(Pool & pool) {
    srand(20031006); // same seed, same data.
    for(int i = 0; i < UNI_DATA_SIZE; i++) {
        tc[i].zeroToOne  =  rand() % 2;
        tc[i].raw_amount =  (uint160(1) << i);
        tc[i].amount     =  tc[i].raw_amount.ToDouble();
        std::pair<int256, int256> ret = raw_swap_handle(pool, tc[i].zeroToOne, tc[i].raw_amount);
        tc[i].result     = make_pair(ret.first.ToDouble(), ret.second.ToDouble());
    }
}


int main(){
    cerr << "Initializing tick." << endl;
    initializeTicksPrice();
    cerr << "Generating data." << endl;

    Pool pool("pool_state");
    generate(pool);
    cerr << "done" << endl;

    double timer = clock(); int uniq_id;
    for(int i = 0; i < TEST_TIME; i++) {
        uniq_id = i % UNI_DATA_SIZE;
        result[uniq_id] = swap_handle(pool, tc[uniq_id].zeroToOne, tc[uniq_id].amount);
    }
    timer = (clock() - timer) / CLOCKS_PER_SEC * 1000 * 1000 * 1000;
    timer /= TEST_TIME;

    for(int i = 0; i < UNI_DATA_SIZE; i++) {
        pair<double, double> ret1 = result[i], ret0 = tc[i].result;
        bool zeroToOne = tc[i].zeroToOne;
        double amountSpecified = tc[i].amount;

        double diffe;
        if (zeroToOne^(amountSpecified > 0)) {
            diffe = fabs(  (ret1.second - ret0.second) / std::max(fabs(ret0.second), fabs(ret1.second))  );
        } else {
            diffe = fabs(  (ret1.first - ret0.first) / std::max(fabs(ret0.first), fabs(ret1.first))  );
        }
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

    printf("Time used       = %.5lf ns\n", timer);
    printf("Maximum mistake  = %.5lf\n", MAX_DIFF);
    printf("Average mistake  = %.5lf\n", TOT_DIFF / TOT_CNT);
    return 0;
}
