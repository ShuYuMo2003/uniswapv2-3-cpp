#include <cstdio>
#include <cstdlib>
#include <vector>
#include <ctime>
#include <climits>

#include "include/types.h"
#include "include/pool.h"
#include "include/regression.h"

const double MAX_DEVIATION = 1e-6;

char buffer[1024 * 1024];

bool DebugOutput;

namespace v3{

enum EventType {SWAP, MINT, BURN, INIT};

double deviation(double a, double b) {
    if(fabs(a - b) < EPS) return 0;
    return fabs((a - b) / std::max(a, b));
}

void db2int256(int256 & dist, double o) {
    unsigned long long src;
    assert(o >= 0);
    if(o > ULONG_LONG_MAX)  src = ULONG_LONG_MAX;
    else src = o;
    dist = 0;
#if TTMATH_BITS_PER_UINT == 32
    dist.table[0] = src & ((1ull << 32) - 1);
    dist.table[1] = src >> 32;
#else
    dist.table[0] = src;
#endif
}

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

    std::vector<Lagrange> poly;

    std::vector<std::pair<unsigned int, double> > sampleTick;

    void * mallocPool(size_t size) {
        return malloc(size);
    }
    void freePool(void * o) {
        free(o);
    }
    void InitSampleTick(){
        const int TickCnt = 26;
        const double RangeL = -3.9;
        const double RangeR = 0.9;

        const int MinAmount = 1;
        const int MaxAmount = 5e4;
        std::vector<double> sampleTmp;
        sampleTmp.clear();
        sampleTick.clear();
        for(double x = RangeL; x <= RangeR; x += (RangeR - RangeL) / TickCnt) {
            sampleTmp.push_back(exp(x));
        }
        double rate = (MaxAmount - MinAmount) / (*(sampleTmp.end() - 1) - *sampleTmp.begin());
        for(double now : sampleTmp) {
            sampleTick.push_back(std::make_pair((now - *sampleTmp.begin()) * rate + MinAmount, 0));
        }
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




    V3Pool(int fee, int tickSpacing, uint256 maxLiquidityPerTick) {
        InitSampleTick();
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

    FloatType __querySwapFloat(bool zeroToOne, double amountIn) {
        static FloatType SQPRL = uint160("4295128740").X96ToDouble();
        static FloatType SQPRR = uint160("1461446703485210103287273052203988822378723970341").X96ToDouble();
        return zeroToOne ? swap(FloatPool, zeroToOne, amountIn, zeroToOne ? SQPRL : SQPRR, false).second
                         : swap(FloatPool, zeroToOne, amountIn, zeroToOne ? SQPRL : SQPRR, false).first;
    }

    int256 __querySwapInt(bool zeroToOne, int256 amountIn) {
        static uint160 SQPRL = uint160("4295128740");
        static uint160 SQPRR = uint160("1461446703485210103287273052203988822378723970341");
        return zeroToOne ? swap(IntPool, zeroToOne, amountIn, zeroToOne ? SQPRL : SQPRR, false).second
                         : swap(IntPool, zeroToOne, amountIn, zeroToOne ? SQPRL : SQPRR, false).first;
    }
    /*
    // 类似于自适应辛普森
    void makeRegressionRecursion(int256 & xL,
                                 int256 & yL,
                                 int256 & xR,
                                 int256 & yR,
                                 const bool & zeroToOne)
        {
        if(xR - xL <= 30) return ;

        int256 midx = (xL + xR) / 2;
        int256 midy = __querySwapInt(zeroToOne, midx);
        int256 * (samplex[3]) = {&xL, &midx, &xR};
        int256 * (sampley[3]) = {&yL, &midy, &yR};
        Regression_t model;

        BuildRegression(samplex, sampley, 3, &model);

        double midDivation = deviation(evaluate(&model, midx), midy.ToDouble());
        if(midDivation <= MAX_DEVIATION) {
            model.upper = xR.ToDouble();
            reg.push_back(model);
        } else if(midDivation < 1e-4) { // 嫌慢就调这个.
            makeRegressionRecursion(xL, yL, midx, midy, zeroToOne);
            makeRegressionRecursion(midx, midy, xR, yR, zeroToOne);
        } //else makeRegressionRecursion(xL, yL, midx, midy, zeroToOne);
        // else pls get out, shit range.
    }

    void buildRegressionModel(){
        regressionReady = false;
        // `体量小币种` 换 `体量大币种` 时，才使用拟合.
        int zeroToOne = IntPool->slot0.sqrtPriceX96 > 1;

        #define __MinAmount__0 10
        #define __MinAmount__1 70

        static int256 MaxAmount = 0, MinAmount = __MinAmount__0, yL, yR;
        reg.clear();
        static double MaxAmountDb = 0;

        // 实验可知， AmountIn 和 deviation 成严格反比。
        MaxAmountDb = deviation( __querySwapFloat(zeroToOne, __MinAmount__0),
                                (yL = __querySwapInt(zeroToOne, __MinAmount__0)).ToDouble()
                        ) * __MinAmount__0 / MAX_DEVIATION;

        MaxAmountDb +=deviation( __querySwapFloat(zeroToOne, __MinAmount__1),
                                (yL = __querySwapInt(zeroToOne, __MinAmount__1)).ToDouble()
                        ) * __MinAmount__1 / MAX_DEVIATION;

        MaxAmountDb /= 2; // 取平均 降低扰动。

        MaxAmount = (unsigned int)MaxAmountDb;
        // std::cerr << MaxAmount.ToDouble() << " " << MaxAmountDb << std::endl;

        if(MaxAmount - MinAmount <= 10)
            return ;

        yR = __querySwapInt(zeroToOne, MaxAmount);

        if(DebugOutput)
            std::cerr << "New one " << MinAmount << " " << yL << " " << MaxAmount << " " << yR << std::endl;

        makeRegressionRecursion(MinAmount, yL, MaxAmount, yR, zeroToOne);

        if(reg.size() > 0){
            regUpperLimit = reg[reg.size() - 1].upper;
            regressionReady = true;
        }
    }
    */

   void buildRegressionModel() {
        // std::cerr << "Build New Regrission" << std::endl;
        bool zeroToOne = IntPool->slot0.sqrtPriceX96 > 1;



        for(auto & now : sampleTick) {
            now.second = __querySwapInt(zeroToOne, now.first).ToDouble();
        }

        poly.clear();
        Lagrange now;
        for(int i = 2; i < sampleTick.size(); i += 2) {
            now.init(sampleTick, i - 3, i + 1, i);
            poly.push_back(now);
        }

        // std::cerr << "================== Ticks ==================" << std::endl;
        // for(auto now : poly) {
        //     std::cerr << now.Upper << std::endl;
        // }
        // std::cerr << "================== Ticks ==================" << std::endl;
   }
   V3Pool(Pool<false> * o) {
        InitSampleTick();
        FloatPoolSize = 0; FloatPool = NULL;
        size_t oldPoolSize = sizeOfPool(o);
        IntPoolSize = oldPoolSize << 1;
        IntPool = (Pool<false> *)mallocPool(IntPoolSize);
        memcpy(IntPool, o, oldPoolSize);
        sync();
        buildRegressionModel();
    }

    double query(bool zeroToOne, double amountIn) {
        static FloatType SQPRL = uint160("4295128740").X96ToDouble();
        static FloatType SQPRR = uint160("1461446703485210103287273052203988822378723970341").X96ToDouble();
        if(zeroToOne == (IntPool->slot0.sqrtPriceX96 > 1) && amountIn <= (poly.end() - 1)->Upper) {
            static Lagrange temp;
            temp.Upper = amountIn;
            auto aim = lower_bound(poly.begin(), poly.end(), temp);
            assert(aim != poly.end());
            return (*aim)(amountIn);
        } else {
            return zeroToOne ? swap(FloatPool, zeroToOne, amountIn, zeroToOne ? SQPRL : SQPRR, false).second
                             : swap(FloatPool, zeroToOne, amountIn, zeroToOne ? SQPRL : SQPRR, false).first;
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

        if(type != INIT) {
            buildRegressionModel();
        }
    }
};

std::pair<V3Event, bool> tempReadEventsFile(std::istream & is) {
    static char opt[100];

    std::string temp;

    V3Event result;

    if(is >> opt) ; else return std::make_pair(result, true);

    // std::cerr << "Processing " << opt << std::endl;
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
    return std::make_pair(result, false);
}

}

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
        for(int256 amount = 10; amount < upper0 / 2; (amount *= 12) /= 10) {
            testcase now;
            now.zeroToOne  =  1;
            now.raw_amount =  amount;
            now.amount     =  now.raw_amount.ToDouble();
            int256 ret = o.__querySwapInt(now.zeroToOne, now.raw_amount);
            now.raw_result = ret;
            now.result     = ret.ToDouble();
            tc.push_back(now);
        }
        for(int256 amount = 10; amount < upper1 / 2; (amount *= 12) /= 10) {
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
        std::cerr << "Generating test data" << std::endl;
        generateFromExpon(o, upper0, upper1);
        std::cerr << "Generating test data done" << std::endl;
        double timer[UNI_DATA_SIZE];
        for(int t = 0; t < UNI_DATA_SIZE; t++) {
            // std::cerr << "Testing t = " << t << std::endl;
            timer[t] = clock();
            for(int i = 0; i < TEST_TIME; i++) {
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


int main(int argc, char * argv){
    std::cerr << std::setiosflags(std::ios::fixed) << std::setprecision(20);
    std::cout << std::setiosflags(std::ios::fixed) << std::setprecision(1);
    initializeTicksPrice();
    std::ifstream fin("events/main_pool");
    int fee; int tickSpacing; uint256 maxLiquidityPerTick;
    fin >> fee >> tickSpacing >> maxLiquidityPerTick;
#ifdef HANDLE_EVENTS
    V3Pool pool(fee, tickSpacing, maxLiquidityPerTick);
    std::vector<V3Event> data;
    int tot = 0;

    std::cerr << "Load events" << std::endl;
    while(true) {
        auto [even, eof] = v3::tempReadEventsFile(fin);
        if(eof) break;
        data.push_back(even);
    }
    std::cerr << "Load events done." << std::endl;

    double Timer = clock();
    for(int i = 0; i < data.size(); i++) {
        pool.processEvent(data[i]);
        if((i & ((1 << 14) - 1)) == 0) std::cerr << "Handle " << i << std::endl;
    }
    Timer = (clock() - Timer) / CLOCKS_PER_SEC * 1000 * 1000 * 1000;
    Timer /= (data.size());
    std::cerr << "mean of process event time used = " << Timer << " ns\n";
#else
    FILE * fptr = fopen("pool_state", "rb");
    fread(buffer, 1, sizeof(buffer), fptr);
    V3Pool pool((Pool<false>*)buffer);
#endif


    Test::Test(pool, int256("94033269757636"), int256("65308223357628934551218"));
    SavePool(pool.IntPool, "pool_state");
    return 0;
}