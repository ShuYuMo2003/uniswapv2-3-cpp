#ifndef headerfilev2pair
#define headerfilev2pair
#include "types.h"
#include <mutex>
#include "../lib/ttmath/ttmathint.h"
#include "../lib/ttmath/ttmathuint.h"


namespace v2{

enum EventType {CRET, SET};

struct V2Event{
    int type;
    std::string address, token0, token1;
    unsigned long long idxHash;
    double reserve0, reserve1;
};


class V2Pair{
public:
    struct PairStatus{
         // 在这个操作之前池子的状态。
        double reserve[2];
        IndexType timestamp;
        PairStatus() { reserve[0] = -1; reserve[1] = -1; }
    };
    std::deque<std::pair<IndexType, PairStatus>> history; // (idxHash, status) 在执行第 idxHash 个 event 之前池子状态为 status

    double reserve[2] = {0};
    std::string token[2];
    std::mutex block;
    std::string pairAddress;
    unsigned long long latestIdxHash;
    V2Pair(const std::string & token0, const std::string & token1, const double & reserve0, const double & reserve1, const unsigned long long & idx, std::string addd) { // 用于创建 Pair.
        token[0] = token0;
        token[1] = token1;
        reserve[0] = reserve0;
        reserve[1] = reserve1;
        latestIdxHash = idx;
        pairAddress = addd;
    }
    void rollbackTo(IndexType limit) { // 恰好执行完第 limit 个 event 的状态。
        if(latestIdxHash <= limit) return ;
        std::lock_guard<std::mutex> lb(block);
        PairStatus lastStatus;
        while(!history.empty()) {
            if(history.back().first <= limit) {
                break;
            } else {
                lastStatus = history.back().second;
                history.pop_back();
            }
        }

        if(lastStatus.reserve[0] != -1) {
            reserve[0] = lastStatus.reserve[0];
            reserve[1] = lastStatus.reserve[1];
            latestIdxHash = lastStatus.timestamp;
        } else { // 创建的 events 被撤回。 标记 Pair 为不可用。
            Logger(std::cout, WARN, "rollbackto") << "Not found backup of " << pairAddress << "(v2 pair), treat as deleting pool." << kkl();
            assert(false); // 目前不监听池子创建指令。
            reserve[0] = -1;
            reserve[1] = -1;
            latestIdxHash = 0;
        }

    }
    void processEvent(const V2Event & e){
        std::lock_guard<std::mutex> lb(block);
        if(e.idxHash != 0u) { // 表示强制更改，不回滚，不检查。
            if(latestIdxHash >= e.idxHash) {
                Logger(std::cout, ERROR, "processEvent") << "detached early event (idx=" << e.idxHash << ", nowIdx=" << latestIdxHash << " ignored. "<< pairAddress << "(v2 pair)" << kkl();
                // assert(false); //TOOD: recover
                return ;
            }

            while(   !history.empty()
                && e.idxHash / LogIndexMask - history.front().first / LogIndexMask > SUPPORT_ROLLBACK_BLOCKS )
                history.pop_front();

            PairStatus temp; temp.reserve[0] = reserve[0]; temp.reserve[1] = reserve[1]; temp.timestamp =  latestIdxHash;
            history.push_back(std::make_pair(e.idxHash, temp));


            latestIdxHash = e.idxHash;
        }

        if(e.idxHash == 0u)
            Logger(std::cout, WARN, "v2 pair") << "Force v2 pair status update on " << pairAddress << kkl();

        assert(e.type == SET);

        if(e.type == SET) {
            reserve[0] = e.reserve0;
            reserve[1] = e.reserve1;
        }

    }
    double query(bool zeroToOne, double amountIn) {
        if(zeroToOne ? amountIn + 10 > reserve[0] : amountIn + 10 > reserve[1])
            return -1;
        amountIn = amountIn * 997;
        double numerator, denominator;
        if (zeroToOne) {
            numerator = amountIn * reserve[1];
            denominator = reserve[0] * 1000 + amountIn;
        } else {
            numerator = amountIn * reserve[0];
            denominator = reserve[1] * 1000 + amountIn;
        }
        return numerator / denominator;
    }
};

V2Event rawdata2event(std::istringstream & is) {
    static char opt[100];

    static std::string temp;
    static int128 tempn;

    V2Event result;

    assert(is >> opt);
    assert(opt[0] == '2');

    if(opt[1] == 'c') { // CREAT
        result.type = CRET;
        is >> result.address >> result.token0 >> result.token1 >> result.idxHash;
    } else if(opt[1] == 's') { // SET
        result.type = SET;
        is >> result.address;
        is >> tempn; result.reserve0 = tempn.ToDouble();
        is >> tempn; result.reserve1 = tempn.ToDouble();
        is >> result.idxHash;
    }
    return result;
}


}

#endif