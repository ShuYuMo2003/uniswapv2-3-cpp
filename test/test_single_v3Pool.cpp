#include <cstdio>
#include <cstdlib>
#include <vector>
#include <ctime>
#include <climits>

#include "../include/v3pool.h"

const double MAX_DEVIATION = 1e-6;


using v3::V3Pool;
using v3::V3Event;

namespace Test{
    int UNI_DATA_SIZE = -1;
    const int TEST_TIME = 1e4;

    int TOT_CNT = 0;
    double MAX_DIFF = -1, TOT_DIFF = 0;
    struct testcase{
        bool zeroToOne;
        int256 raw_amount;
        FloatType amount;
        FloatType result;
        int256 raw_result;
        FloatType dev;
        int repeat;
    };
    std::vector<testcase> tc;
    std::vector<FloatType> result;
    void generateFromExpon(V3Pool & o, int256 upper0, int256 upper1) {
        tc.clear(); UNI_DATA_SIZE = 0;
        for(int256 amount = 10; amount < upper0 / 2; (amount *= 13) /= 10) {
            testcase now;
            now.zeroToOne  =  1;
            now.raw_amount =  amount;
            now.amount     =  now.raw_amount.ToDouble();
            int256 ret = o.__querySwapInt(now.zeroToOne, now.raw_amount);
            now.raw_result = ret;
            now.result     = ret.ToDouble();
            tc.push_back(now);
        }
        for(int256 amount = 10; amount < upper1 / 2; (amount *= 13) /= 10) {
            testcase now;
            now.zeroToOne  =  0;
            now.raw_amount =  amount;
            now.amount     =  now.raw_amount.ToDouble();
            int256 ret = o.__querySwapInt(now.zeroToOne, now.raw_amount);
            now.raw_result = ret;
            now.result     = ret.ToDouble();
            tc.push_back(now);
        }
        UNI_DATA_SIZE = tc.size();
        result.resize(UNI_DATA_SIZE);
    }
    std::tuple<std::string, std::string, std::string, std::string> split(double a, double b) {
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
        return std::make_tuple(a0, a1, b0, b1);
    }
    std::string whitespace(int n) {
        std::string ret = "";
        if(n < 0) return ret;
        while(n--)ret.push_back(' ');
        return ret;
    }
    void Test(V3Pool & o, int256 upper0, int256 upper1){

        // std::cerr << "===================== FLOAT POOL ===================== " << std::endl;
        // auto ret0 = o.__querySwapFloat(1, 82256);
        // std::cerr << "===================== BIGIN POOL ===================== " << std::endl;
        // auto ret1 = o.__querySwapInt(1, 82256);
        // std::cerr << "====================================================== " << std::endl;
        // std::cout << ret0 << " " << ret1 << std::endl;

        // return ;

        std::cerr << "Generating test data" << std::endl;
        generateFromExpon(o, upper0, upper1);
        std::cerr << "Generating test data done" << std::endl;
        double timer[UNI_DATA_SIZE];
        for(int t = 0; t < UNI_DATA_SIZE; t++) {
            // std::cerr << "Testing t = " << t << std::endl;
            timer[t] = clock();
            for(int i = 0; i < 1; i++) {
                result[t] = o.query(tc[t].zeroToOne, tc[t].amount);
            }

            timer[t] = (clock() - timer[t]) / CLOCKS_PER_SEC * 1000 * 1000 * 1000;
            timer[t] /= TEST_TIME;
        }

        for(int i = 0; i < UNI_DATA_SIZE; i++) {
            FloatType ret1 = result[i], ret0 = tc[i].result;
            bool zeroToOne = tc[i].zeroToOne;
            FloatType amountSpecified = tc[i].amount;
            FloatType diffe;

            diffe = v3::deviation(ret1, ret0);

            MAX_DIFF = std::max(MAX_DIFF, diffe);
            if(diffe < 0.9){

                TOT_DIFF += diffe; TOT_CNT ++;
            }
            tc[i].dev = diffe;
        }
        printf("D |                 AmountIn |Time(ns)| Deviation  |        Standard AmountOut | fuck AmountOut\n");
        for(int t = 0; t < UNI_DATA_SIZE; t++) {
            std::string a0, a1, b0, b1;
            std::tie(a0, a1, b0, b1) = split(tc[t].result, result[t]);
            printf("%d | % 24.0lf | % 6.0lf | %.8lf | %s%s\033[41;30m%s\033[0m | %s\033[41;30m%s\033[0m \n", tc[t].zeroToOne, tc[t].amount, timer[t], tc[t].dev, whitespace(25 - (a0.size() + a1.size())).c_str(), a0.c_str(), a1.c_str(), b0.c_str(), b1.c_str());
        }
        printf("Maximum mistake  = %.30lf\n", MAX_DIFF);
    }
}
#define HANDLE_EVENTS

int main(){
    std::cerr << std::setiosflags(std::ios::fixed) << std::setprecision(20);
    std::cout << std::setiosflags(std::ios::fixed) << std::setprecision(1);
    initializeTicksPrice();
    std::ifstream fin("test_events");
    int fee; int tickSpacing; uint256 maxLiquidityPerTick;
    fin >> fee >> tickSpacing >> maxLiquidityPerTick;
#ifdef HANDLE_EVENTS
    V3Pool pool(fee, tickSpacing, maxLiquidityPerTick, std::string("QAQ"), std::string("QWQ"), 0ull);
    // std::vector<V3Event> data;
    // int tot = 0;

    std::cerr << "Load events" << std::endl; int t = 0;
    while(true) {
        std::cerr << (++t) << ' ';
        auto [even, eof] = v3::tempReadEventsFile(fin);
        if(eof) break;
        pool.processEvent(even);
    }
    std::cerr << "Load events done." << std::endl;

    // double Timer = clock();
    // for(int i = 0; i < data.size(); i++) {

    //     std::cerr << "Handle " << i << std::endl;
    // }
    // Timer = (clock() - Timer) / CLOCKS_PER_SEC * 1000 * 1000 * 1000;
    // Timer /= (data.size());
    // std::cerr << "mean of process event time used = " << Timer << " ns\n";
#else
    FILE * fptr = fopen("../pool_state/0x7BeA39867e4169DBe237d55C8242a8f2fcDcc387.ip", "rb");
    if(fptr == NULL) { std::cerr << "QAQ" << std::endl; }
    fread(v3::buffer, 1, sizeof(v3::buffer), fptr);
    V3Pool pool((Pool<false>*)v3::buffer);
    std::cerr << "read done" << std::endl;
#endif
    // auto rawevent = std::string("swap 0x88e6A0c2dDD26FEEb64F039a2c41296FcB3f5640 0 230000000000000000 1339939048927153956667167281516906 -803725510 230000000000000000 734368184554852844 194725");
    // std::istringstream irawevent(rawevent);
    // v3::V3Event testev = v3::rawdata2event(irawevent);
    // pool.processEvent(testev);


    Test::Test(pool, int256("78572000000000"), int256("78572000000000"));

    SavePool(pool.IntPool, "pool_state");
    return 0;
}