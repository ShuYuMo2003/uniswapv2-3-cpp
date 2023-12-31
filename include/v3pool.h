#ifndef headfilev3pool
#define headfilev3pool
#include <cstdio>
#include <vector>
#include <climits>
#include <mutex>
#include <deque>

#include "types.h"
#include "pool.h"
#include "regression.h"
#include "logger.h"



namespace v3{
char buffer[5 * 1024 * 1024];
enum EventType {SWAP, MINT, BURN, INIT, CRET};

double deviation(double a, double b) {
    if(fabs(a - b) < EPS) return 0;
    return fabs((a - b) / std::max(a, b));
}

void db2int256(int256 & dist, double o) {
    unsigned long long src;
    assert(o >= 0);
    if(o > std::numeric_limits<unsigned long long>::max())  src = std::numeric_limits<unsigned long long>::max();
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
    std::string address, token0, token1;
    bool zeroToOne;
    int tick;
    int tickLower;
    int tickUpper;
    int256 amount;
    unsigned long long idxHash; // A kind of orderable hash value: BlockNumber + LogIndex. (LogIndex may start with serval leading zero to make the length is exactly 5.)

    uint160 sqrtPrice;
    int256 ramount0;
    int256 ramount1;
    uint128 liquidity;
    int fee, tickspace;
};

// const int BLOCKER_CNT_MAX = 1e5 + 100;
// std::mutex EdgeBlocker[BLOCKER_CNT_MAX];
// int block_cnt = 0;

struct V3Pool{
    struct PoolStatus{
         // 在这个操作之前池子的状态。
        IndexType timestamp;
        void * buffer;
        bool initialized;
        PoolStatus(){ buffer = NULL; }
    };
    std::deque<std::pair<IndexType, PoolStatus>> history; // (idxHash, status) 在执行第 idxHash 个 event 之前池子状态为 status
    std::string poolAddress;
    Pool<false> * IntPool;
    size_t IntPoolSize;
    Pool<true>  * FloatPool;
    size_t FloatPoolSize;
    bool initialized = false; // ready for handling query from algo.

    std::vector<Lagrange> poly[2];

    std::vector<std::pair<unsigned int, double> > sampleTick[2];

    std::mutex block;
    IndexType latestIdxHash;

    std::string token[2];

    void * mallocPool(size_t size) {
        return malloc(size);
    }
    void freePool(void * o) {
        free(o);
    }
    void save(std::string filename) {
        SavePool(IntPool, filename);
    }
    void InitSampleTick(){
        const int TickCnt = 26;
        const double LowerAmountOut = 1e9;
        const double RangeL = -3.9;
        const double RangeR = 0.9;
        static std::vector<double> sampleTmp;
        if(!sampleTmp.size()) {
            sampleTmp.clear();
            for(double x = RangeL; x <= RangeR; x += (RangeR - RangeL) / TickCnt) {
                sampleTmp.push_back(exp(x));
            }
        }
        double SQ2 = pow(IntPool->slot0.sqrtPriceX96.X96ToDouble(), 2);

        for(int zeroToOne = 0; zeroToOne <= 1; zeroToOne++) {
            double MinAmount = 1;
            double MaxAmount = (zeroToOne ? 1 / SQ2 : SQ2) * LowerAmountOut;
            MaxAmount = std::max(MaxAmount, 1e4);
            MaxAmount = std::min(MaxAmount, (double)INT_MAX);
            // std::cerr << "MaxAmout = " << MaxAmount << std::endl;

            sampleTick[zeroToOne].clear();

            double rate = (MaxAmount - MinAmount) / (*(sampleTmp.end() - 1) - *sampleTmp.begin());
            for(double now : sampleTmp) {
                sampleTick[zeroToOne].push_back(std::make_pair((now - *sampleTmp.begin()) * rate + MinAmount, 0));
            }
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

        memcpy((void * )FloatPool, buffer, PoolSize);
    }
    V3Pool(int fee, int tickSpacing, uint256 maxLiquidityPerTick, std::string token0, std::string token1, unsigned long long idxhash, std::string add) {
        FloatPoolSize = 0; FloatPool = NULL; token[0] = token0; token[1] = token1; latestIdxHash = idxhash;
        poolAddress = add;
        Pool<false> temppool(fee, tickSpacing, maxLiquidityPerTick);
        size_t poolsize = sizeOfPool(&temppool);
        IntPoolSize = poolsize << 1;
        IntPool = (Pool<false>*)mallocPool(IntPoolSize);
        memcpy((void *)IntPool, &temppool, poolsize);
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
            memcpy((void *)newIntPool, IntPool, PoolSize);
            freePool(IntPool);
            IntPool = newIntPool;
        }
    }
    FloatType __querySwapFloat(bool zeroToOne, double amountIn) {
        static FloatType SQPRL = uint160("4295128740").X96ToDouble();
        static FloatType SQPRR = uint160("1461446703485210103287273052203988822378723970341").X96ToDouble();
        auto ret = swap(FloatPool, zeroToOne, amountIn, zeroToOne ? SQPRL : SQPRR, false);
        return zeroToOne ? -ret.second
                         : -ret.first;
    }

    int256 __querySwapInt(bool zeroToOne, int256 amountIn) {
        static uint160 SQPRL = uint160("4295128740");
        static uint160 SQPRR = uint160("1461446703485210103287273052203988822378723970341");
        // std::cerr << zeroToOne << " " << amountIn << " " << (zeroToOne ? SQPRL : SQPRR) << std::endl;
        auto ret = swap(IntPool, zeroToOne, amountIn, zeroToOne ? SQPRL : SQPRR, false);
        // cerr << ""
        return zeroToOne ? -ret.second
                         : -ret.first;
    }

    int regressionRebuildTime = 0;
    void buildRegressionModel() {
        // std::cerr << "Build New Regrission" << std::endl;
        if((regressionRebuildTime++) % 50 == 0) InitSampleTick();

        for(int zeroToOne = 0; zeroToOne <= 1; zeroToOne ++) {
            // std::cerr << "Testing Build" << std::endl;

            for(auto & now : sampleTick[zeroToOne]) {
                // std::cerr << "Querying point " << zeroToOne << " " << now.first << std::endl;
                now.second = __querySwapInt(zeroToOne, now.first).ToDouble();
            }

            poly[zeroToOne].clear();
            Lagrange now;
            for(int i = 2; i < sampleTick[zeroToOne].size(); i += 2) {
                now.init(sampleTick[zeroToOne], i - 3, i + 1, i);
                poly[zeroToOne].push_back(now);
            }

            // std::cerr << "================== Ticks ==================" << std::endl;
            // for(auto now : poly) {
            //     std::cerr << now.Upper << std::endl;
            // }
            // std::cerr << "================== Ticks ==================" << std::endl;
        }
   }
    V3Pool(Pool<false> * o, std::string token0, std::string token1, unsigned long long idxhash, bool __init, std::string addd) {
        FloatPoolSize = 0; FloatPool = NULL; token[0] = token0; token[1] = token1; latestIdxHash = idxhash;
        size_t oldPoolSize = sizeOfPool(o);
        IntPoolSize = oldPoolSize << 1;
        IntPool = (Pool<false> *)mallocPool(IntPoolSize);
        memcpy((void*)IntPool, o, oldPoolSize);
        sync();
        initialized = __init;
        if(initialized) buildRegressionModel();
        poolAddress = addd;
    }
    double query(bool zeroToOne, double amountIn) {
        std::lock_guard<std::mutex> lb(block);
        static FloatType SQPRL = uint160("4295128740").X96ToDouble();
        static FloatType SQPRR = uint160("1461446703485210103287273052203988822378723970341").X96ToDouble();
        if(IntPool->liquidity < 1000)
            return -6;
        if(!initialized)
            return -5;
        if(!poly[zeroToOne].size())
            return -4;
        if(amountIn < 10)
            return -3;
        // std::cerr << "query amount = " << amountIn << " direction = " << zeroToOne << std::endl;
        if(amountIn <= (poly[zeroToOne].end() - 1)->Upper) {
            Lagrange temp;
            temp.Upper = amountIn;
            auto aim = lower_bound(poly[zeroToOne].begin(), poly[zeroToOne].end(), temp);
            assert(aim != poly[zeroToOne].end());
            // std::cerr << "Use regression result = " << (*aim)(amountIn) << std::endl;
            return (*aim)(amountIn);
        } else {
            // std::cerr << "swap(" << zeroToOne << ", " << amountIn << ", " << (zeroToOne ? SQPRL : SQPRR) << ") = ";
            auto result = swap(FloatPool, zeroToOne, amountIn, zeroToOne ? SQPRL : SQPRR, false);
            // std::cerr << result.first << ", " << result.second << std::endl;
            // std::cerr << "Use swap" << std::endl;
            if(zeroToOne) {
                if(fabs(result.first - amountIn) < EPS) return -result.second;
                else return -2;
            } else {
                if(fabs(result.second - amountIn) < EPS) return -result.first;
                else return -1;
            }
        }
    }

    void rollbackTo(IndexType limit) { // 恰好执行完第 limit 个 event 的状态。
        if(latestIdxHash <= limit) return ;
        std::lock_guard<std::mutex> lb(block);
        // Logger(std::cout, ERROR, "rollbackTo") << "now at " << latestIdxHash << " rollback to " << limit << kkl(); ////
        PoolStatus lastStatus;
        while(!history.empty()) {
            if(history.back().first <= limit) {
                break;
            } else {
                if(lastStatus.buffer != NULL)
                    free(lastStatus.buffer);
                lastStatus = history.back().second;
                history.pop_back();
            }
        }
        if(lastStatus.buffer != NULL) {
            // Logger(std::cout, INFO, "rollbackto") << "Found backup idx = " << lastStatus.timestamp << kkl(); ////
            if(FloatPool != NULL) free(FloatPool);
            if(IntPool != NULL)  free(IntPool);

            FloatPoolSize = 0; FloatPool = NULL;
            latestIdxHash = lastStatus.timestamp;

            size_t oldPoolSize = sizeOfPool((Pool<false> *)lastStatus.buffer);
            IntPoolSize = oldPoolSize << 1;
            IntPool = (Pool<false> *)mallocPool(IntPoolSize);
            memcpy((void*)IntPool, lastStatus.buffer, oldPoolSize);
            sync();
            initialized = lastStatus.initialized;
            if(initialized) buildRegressionModel();

            free(lastStatus.buffer);
        } else { // 认为创建的 events 被撤回。 标记 Pool 为不可用。
            Logger(std::cout, WARN, "rollbackto") << "Not found backup of " << poolAddress << "(v3 pool), treat as deleting pool." << kkl();
            assert(false); // 目前不监听池子创建指令。
            initialized = false;
            latestIdxHash = 0;
        }
    }

    void processEvent(V3Event & e){
        std::lock_guard<std::mutex> lb(block);
        if(e.idxHash != 0u) {
            if(latestIdxHash >= e.idxHash) {
                Logger(std::cout, ERROR, "processEvent") << "detached early event (idx=" << e.idxHash << ", nowIdx=" << latestIdxHash << " ignored. "<< poolAddress << "(v3 pool)" << kkl();
                //TOOD: recover
                // assert(false);
                return ;
            }

            while(   !history.empty()
                && e.idxHash / LogIndexMask - history.front().first / LogIndexMask > SUPPORT_ROLLBACK_BLOCKS )
                history.pop_front();


            PoolStatus nowDataStatus;
            nowDataStatus.initialized = initialized;
            nowDataStatus.timestamp   = latestIdxHash;
            nowDataStatus.buffer      = malloc(IntPoolSize);
            CopyPool(IntPool, (Pool<false> * )nowDataStatus.buffer);
            history.push_back(std::make_pair(e.idxHash, nowDataStatus));
        }

        // std::cerr << "Processing" << std::endl;
        int type = e.type;

        static int256 ramount0, ramount1;

        if(type == SWAP) {
            if(e.ramount0 == 0 && e.ramount1 == 0) {
                IntPool->liquidity = e.liquidity;
                IntPool->slot0.tick = e.tick;
                IntPool->slot0.sqrtPriceX96 = e.sqrtPrice;
                goto END;
            }

            static uint160 SQPRL = uint160("4295128740");
            static uint160 SQPRR = uint160("1461446703485210103287273052203988822378723970341");

            bool success = false;
            memcpy(buffer, IntPool, IntPoolSize);

            // swap case 1.
            if(!success && e.amount != 0) {
#ifdef VALIDATE
                double __SQ = e.zeroToOne ? SQPRL.X96ToDouble() : SQPRR.X96ToDouble(), __am = e.amount.ToDouble();
                std::pair<double, double> __FloatResult;
                if(__am > 0) __FloatResult = swap(FloatPool, e.zeroToOne, __am,__SQ, false);
#endif
                std::tie(ramount0, ramount1) = swap(IntPool,
                                                    e.zeroToOne,
                                                    e.amount,
                                                    e.zeroToOne ? SQPRL
                                                                : SQPRR,
                                                    true);
#ifdef VALIDATE
                if(__am > 0 || (deviation(__FloatResult.first, ramount0.ToDouble()) < 1e-3 && deviation(__FloatResult.second, ramount1.ToDouble()) < 1e-3));
                else {
                    std::cerr << e.address << ": " << e.zeroToOne << " " << e.amount<< std::endl;
                    std::cerr << __FloatResult.first << " " << ramount0 << std::endl;
                    std::cerr << __FloatResult.second << " " << ramount1 << std::endl;
                    assert(false);
                }
#endif
                // std::cerr << "================================== validate info 1 ==================================" << std::endl;
                // std::cerr << ramount0 << " " << e.ramount0 << std::endl;
                // std::cerr << ramount1 << " " << e.ramount1 << std::endl;
                // std::cerr << IntPool->liquidity << " " << e.liquidity << std::endl;
                // std::cerr << IntPool->slot0.tick << " " << e.tick << std::endl;
                // std::cerr << IntPool->slot0.sqrtPriceX96 << " " << e.sqrtPrice << std::endl;
                // std::cerr << "================================== END END END ==================================" << std::endl;
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
                memcpy((void *)IntPool, buffer, IntPoolSize);
#ifdef VALIDATE
                double __am = temp_amount.ToDouble(), __SQ = e.zeroToOne ? SQPRL.X96ToDouble() : SQPRR.X96ToDouble();
                std::pair<double, double> __FloatResult;
                if(__am > 0) swap(FloatPool, e.zeroToOne, __am, __SQ, false);
#endif
                std::tie(ramount0, ramount1) = swap(IntPool,
                                                    e.zeroToOne,
                                                    temp_amount,
                                                    e.zeroToOne ? SQPRL
                                                                : SQPRR,
                                                    true);
#ifdef VALIDATE
                if(__am < 0 || (deviation(__FloatResult.first, ramount0.ToDouble()) < 1e-3 && deviation(__FloatResult.second, ramount1.ToDouble()) < 1e-3));
                else {
                    std::cerr << e.address << ": " << e.zeroToOne << " " << temp_amount << std::endl;
                    std::cerr << __FloatResult.first << " " << ramount0 << std::endl;
                    std::cerr << __FloatResult.second << " " << ramount1 << std::endl;
                    assert(false);
                }
#endif
                // std::cerr << "================================== validate info 2 ==================================" << std::endl;
                // std::cerr << ramount0 << " " << e.ramount0 << std::endl;
                // std::cerr << ramount1 << " " << e.ramount1 << std::endl;
                // std::cerr << IntPool->liquidity << " " << e.liquidity << std::endl;
                // std::cerr << IntPool->slot0.tick << " " << e.tick << std::endl;
                // std::cerr << IntPool->slot0.sqrtPriceX96 << " " << e.sqrtPrice << std::endl;
                // std::cerr << "================================== END END END ==================================" << std::endl;

                success = (ramount0 == e.ramount0
                        && ramount1 == e.ramount1
                        && IntPool->liquidity == e.liquidity
                        && IntPool->slot0.tick == e.tick
                        && IntPool->slot0.sqrtPriceX96 == e.sqrtPrice);
            }
            // std::cerr << "QAQ" << std::endl;
            // swap case 3.
            if(!success) {
                memcpy((void *)IntPool, buffer, IntPoolSize);
                std::tie(ramount0, ramount1) = swap(IntPool,
                                                    e.zeroToOne,
                                                    e.amount * 10,
                                                    e.sqrtPrice,
                                                    true);
                // std::cerr << "================================== validate info 3 ==================================" << std::endl;
                // std::cerr << ramount0 << " " << e.ramount0 << std::endl;
                // std::cerr << ramount1 << " " << e.ramount1 << std::endl;
                // std::cerr << IntPool->liquidity << " " << e.liquidity << std::endl;
                // std::cerr << IntPool->slot0.tick << " " << e.tick << std::endl;
                // std::cerr << IntPool->slot0.sqrtPriceX96 << " " << e.sqrtPrice << std::endl;
                // std::cerr << "================================== END END END ==================================" << std::endl;
                success = (ramount0 == e.ramount0
                        && ramount1 == e.ramount1
                        && IntPool->liquidity == e.liquidity
                        && IntPool->slot0.tick == e.tick
                        && IntPool->slot0.sqrtPriceX96 == e.sqrtPrice);
            }
            if(not success) {
                std::cout << e.address << " " << e.amount << " " << latestIdxHash << " " << e.idxHash << std::endl;
                assert(false);
            }
        } else if(type == MINT) {
            initialized = true;
            std::tie(ramount0, ramount1) = mint(IntPool,
                                                  e.tickLower,
                                                  e.tickUpper,
                                                  e.amount);
            if(ramount0 == e.ramount0 && ramount1 == e.ramount1); else {
                std::cerr << ramount0 << " " << ramount1 << std::endl;
                std::cout << e.address << " " << e.tickLower << " " << e.tickUpper << " " << e.ramount0 << " " << e.ramount1 << " " << e.idxHash << std::endl;
                assert(false);
            }
            maintainIntPool();
        } else if(type == BURN) {
            std::tie(ramount0, ramount1) = burn(IntPool,
                                                  e.tickLower,
                                                  e.tickUpper,
                                                  e.amount);
            if(ramount0 == e.ramount0 && ramount1 == e.ramount1); else {
                std::cerr << ramount0 << " " << ramount1 << std::endl;
                std::cout << e.address << " " << e.tickLower << " " << e.tickUpper << " " << e.ramount0 << " " << e.ramount1 << " " << e.idxHash << std::endl;
                assert(false);
            }
            maintainIntPool();
        } else if(type == INIT) {
            int nowTick = initialize(IntPool, e.sqrtPrice);
            assert(nowTick == e.tick);
        }
END:
        sync();

        if(type != INIT && initialized) {
            buildRegressionModel();
        }

        latestIdxHash = e.idxHash;
    }
};

V3Event rawdata2event(std::istringstream & is) {

    // std::cerr << "converting" << std::endl;
    static char opt[100];

    std::string temp;

    V3Event result;

    assert(is >> opt);

    // std::cerr << opt << std::endl;

    if(opt[0] == 'i') { // initialize
        is >> result.address;
        result.type = INIT;
        is >> temp;   result.sqrtPrice = temp;
        is >> result.tick;
    } else if(opt[0] == 's') { // swap
        is >> result.address;
        result.type = SWAP;
        is >> result.zeroToOne;
        is >> temp; result.amount = temp;
        is >> temp; result.sqrtPrice = temp;
        is >> temp; result.ramount0 = temp;
        is >> temp; result.ramount1 = temp;
        is >> temp; result.liquidity = temp;
        is >> result.tick;
    }else if(opt[0] == 'b' || opt[0] == 'm') { // burn | mint
        is >> result.address;
        result.type = opt[0] == 'b' ? BURN : MINT;
        is >> result.tickLower >> result.tickUpper;
        is >> temp; result.amount = temp;
        is >> temp; result.ramount0 = temp;
        is >> temp; result.ramount1 = temp;
    }else if(opt[0] == 'p') { // poolcreated.
        result.type = CRET;
        is >> result.token0;
        is >> result.token1;
        is >> result.fee;
        is >> result.tickspace;
        is >> result.liquidity;
        is >> result.address;
    } else assert(false);
    is >> result.idxHash;

    return result;
}

std::pair<V3Event, bool> tempReadEventsFile(std::istream & is) {
    static char opt[100];

    std::string temp;

    V3Event result;

    if(is >> opt) ; else return std::make_pair(result, true);
    std::cerr << "Read Opt = " << opt << std::endl;

    is >> result.address;

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
    if(opt[0] == 'c') { // createdPool.
        is >> temp;
        is >> temp;
        is >> temp;
        is >> temp;
    }
    is >> result.idxHash;
    return std::make_pair(result, false);
}
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


#endif