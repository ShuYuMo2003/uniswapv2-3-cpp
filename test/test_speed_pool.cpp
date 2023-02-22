
#include <iostream>
#include <cstdio>
#include <ctime>
#include <cstring>
#include <cstdlib>
#include "../include/types.h"
#include "../include/pool.h"

using namespace std;

int UNI_DATA_SIZE = -1;
const int TEST_TIME = 3e4;

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
    double dev;
    int repeat;
};
vector<testcase> tc;
vector<pair<double, double>> result;

void generateFromExpon(Pool<false> * pool) {
    for(uint160 amount = 10; amount < uint160("94033269757636"); (amount *= 14) /= 10) {
        testcase now;
        now.zeroToOne  =  1;
        now.raw_amount =  amount;
        now.amount     =  now.raw_amount.ToDouble();
        std::pair<int256, int256> ret = raw_swap_handle(pool, now.zeroToOne, now.raw_amount);
        now.result     = make_pair(ret.first.ToDouble(), ret.second.ToDouble());
        tc.push_back(now);
        // cout << amount << endl;
    }
    for(uint160 amount = 10; amount < uint160("65308223357628934551218"); (amount *= 14) /= 10) {
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
    // random_shuffle(tc.begin(), tc.end());
}

void generateFromEvent(Pool<false> * pool){
    freopen("pool_swap_events", "r", stdin); // data file is generated by `generate_swap_data.py`
    bool zeroToOne; string amount;
    while(cin >> zeroToOne >> amount) {
        testcase now;
        now.zeroToOne  =  zeroToOne;
        now.raw_amount =  amount;
        now.amount     =  now.raw_amount.ToDouble();
        std::pair<int256, int256> ret = raw_swap_handle(pool, now.zeroToOne, now.raw_amount);
        now.result     = make_pair(ret.first.ToDouble(), ret.second.ToDouble());
        tc.push_back(now);
    }
    UNI_DATA_SIZE = tc.size();
    result.resize(UNI_DATA_SIZE);
}

/*
address: 0x88e6A0c2dDD26FEEb64F039a2c41296FcB3f5640
WETH    (0xc02aaa39b223fe8d0a0e5c4f27ead9083c756cc2) (1): 65308223357628934551218
USD Coin(0xa0b86991c6218b36c1d19d4a2e9eb0ce3606eb48) (0): 94033269757636
*/

tuple<std::string, std::string, std::string, std::string> split(double a, double b) {
    static char buffer0[50], buffer1[50];
    sprintf(buffer0, "%.1lf", a);
    sprintf(buffer1, "%.1lf", b);
    std::string a0 = "", a1 = "", b0 = "", b1 = "";
    std::string raw0 = buffer0;
    std::string raw1 = buffer1;
    bool splitit = false;
    int len = std::min(raw0.size(), raw1.size());
    for(int i = 0; i < len; i++) {
        if(raw0[i] == raw1[i] && !splitit) a0.push_back(raw0[i]), b0.push_back(raw1[i]);
        else {
            splitit = true;
            a1.push_back(raw0[i]);
            b1.push_back(raw1[i]);
        }
    }
    while(len < raw0.size()) a1.push_back(raw0[len++]);
    while(len < raw1.size()) b1.push_back(raw1[len++]);
    return make_tuple(a0, a1, b0, b1);
}

std::string whitespace(int n) {
    std::string ret = "";
    if(n < 0) return ret;
    while(n--)ret.push_back(' ');
    return ret;
}

void TestCertainPoint(Pool<false> * pool0, Pool<true> * pool1, int256 amount, bool zeroToOne){
    static uint160 SQPRL = uint160("4295128740");
    static uint160 SQPRR = uint160("1461446703485210103287273052203988822378723970341");
    auto [s_amount0, s_amount1] = swap(pool0, zeroToOne, amount, zeroToOne ? SQPRL : SQPRR, false);
    auto [r_amount0, r_amount1] = swap(pool1, zeroToOne, amount.ToDouble(), zeroToOne ? SQPRL.X96ToDouble() : SQPRR.X96ToDouble(), false);
    cerr << s_amount0 << " " << r_amount0 << endl;
    cerr << s_amount1 << " " << r_amount1 << endl;
}

int main(){
    std::cerr << std::setiosflags(std::ios::fixed) << std::setprecision(7);
    std::cout << std::setiosflags(std::ios::fixed) << std::setprecision(1);
    cerr << "Initializing tick." << endl;
    initializeTicksPrice();
    cerr << "Generating data." << endl;

    register unsigned char MEMPOOL0[256 * 1024];
    register unsigned char MEMPOOL1[256 * 1024];
    Pool<false> *pool       = (Pool<false> *)MEMPOOL0;
    Pool<true>  *pool_float = (Pool<true>  *)MEMPOOL1;

    LoadPool(pool, "pool_state");
    GenerateFloatPool(pool, pool_float);


    generateFromExpon(pool);
    // generateFromEvent(pool);


    cerr << "done generated data cnt = " << UNI_DATA_SIZE << endl;


    // TestCertainPoint(pool, pool_float, int256("94033269757636"), 1);
    // return 0;

    cerr << "run" << endl;
    double timer[UNI_DATA_SIZE];
    for(int t = 0; t < UNI_DATA_SIZE; t++) {
        timer[t] = clock();
        for(int i = 0; i < TEST_TIME; i++) {
            result[t] = swap_handle(pool_float, tc[t].zeroToOne, tc[t].amount);
        }
        timer[t] = (clock() - timer[t]) / CLOCKS_PER_SEC * 1000 * 1000 * 1000;
        timer[t] /= TEST_TIME;
    }
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
        // cerr << "diffe = " << diffe << " " << tc[i].zeroToOne << " "; cout << amountSpecified << " ";
        // cout << "\t" << ret1.first << " " << ret0.first << " \t| " << ret1.second << " " << ret0.second << endl;

        MAX_DIFF = std::max(MAX_DIFF, diffe);
        if(diffe < 0.9){

            TOT_DIFF += diffe; TOT_CNT ++;
        }
        tc[i].dev = diffe;
        // if(diffe < 0.000001 || (ret1.first > -100000 && ret1.first < 100000) || (ret1.second > -100000 && ret1.second < 100000)) ; else {
        //     static char buffer[1000];
        //     sprintf(buffer, "\n\n================================================= FAIL ============================================\n"
        //                     "%.30lf %.30lf\n%.30lf %.30lf\n",
        //             ret0.first, ret0.second, ret1.first, ret1.second);
        //     std::cerr << buffer << std::endl;
        // }
    }
    // printf("======================== Timer ==========================\n");
    printf("D |                 AmountIn |Time(ns)| Deviation  |        Standard AmountOut | fuck AmountOut\n");
    for(int t = 0; t < UNI_DATA_SIZE; t++) {
        std::string a0, a1, b0, b1;
        if(!tc[t].zeroToOne)
            std::tie(a0, a1, b0, b1) = split(tc[t].result.first, result[t].first);
        else
            std::tie(a0, a1, b0, b1) = split(tc[t].result.second, result[t].second);
        printf("%d | % 24.0lf | % 6.0lf | %.8lf | %s%s\033[41;30m%s\033[0m | %s\033[41;30m%s\033[0m \n", tc[t].zeroToOne, tc[t].amount, timer[t], tc[t].dev, whitespace(25 - (a0.size() + a1.size())).c_str(), a0.c_str(), a1.c_str(), b0.c_str(), b1.c_str());
    }
    //printf("======================== Timer ==========================\n");
    printf("Maximum mistake  = %.30lf\n", MAX_DIFF);
    // printf("Average mistake  = %.30f\n", TOT_DIFF / TOT_CNT);

    return 0;
}
