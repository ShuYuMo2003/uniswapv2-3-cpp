#include "include/graph.h"
#include <unordered_map>
#include <set>
#include <cstring>
#include <ctime>
#include <unistd.h>
#include <sw/redis++/redis++.h>

using namespace sw::redis;

const int LogIndexMask = 1e5;

Redis redis = Redis("tcp://127.0.0.1:6379");

std::unordered_map<std::string, v3::V3Pool*> v3Pool;
std::unordered_map<std::string, v2::V2Pair*> v2Pair;
std::set<std::string> lazyUpdatev3Pool, lazyUpdatev2Pair;

unsigned long long recoverStateFromDb(){
    auto idxs = redis.get("UpdatedToBlockNumber");
    unsigned long long LastIdx;
    if(idxs) {
        LastIdx = std::stoull(*idxs) * LogIndexMask;
        std::cout << "[M]   Archived data in db have been found. Last updated to " << (*idxs) << std::endl;
    } else {
        LastIdx = 12369728ull * LogIndexMask;
        redis.set("UpdatedToBlockNumber", std::to_string(12369728ull));
    }


    v3Pool.clear(); v2Pair.clear(); graph::clearEdges();

    std::cout << "Recovering v3 pools .. " << std::endl;
    std::vector<std::string> v3PoolAddress{};
    redis.hkeys("v3poolsData", std::back_inserter(v3PoolAddress));
    for(auto address : v3PoolAddress) {
        auto rawdata = redis.hget("v3poolsData", address);
        auto rawinfo = redis.hget("v3poolsInfo", address);
        assert(rawdata && rawinfo);
        // std::cout << "Got v3 Pool " << address <<" info: " << *rawinfo << std::endl;
        std::istringstream is(*rawinfo);
        static unsigned long long idx; static std::string token0, token1; static bool __init;
        is >> idx >> token0 >> token1 >> __init;
        v3Pool[address] = new v3::V3Pool( (Pool<false> * )(*rawdata).c_str(), token0, token1, idx, __init);
        // std::cout << "Initialized " << address << std::endl;
        graph::addV3Pool(token0, token1, v3Pool[address]);
    }

    std::cout << "Recovering v2 pairs .. " << std::endl;
    std::vector<std::string> v2PairAddress{};
    redis.hkeys("v2pairsData", std::back_inserter(v2PairAddress));
    for(auto address : v2PairAddress) {
        auto raw = redis.hget("v2pairsData", address);
        assert(raw);

        std::istringstream is(*raw);
        static unsigned long long idx, reserve0_L, reserve1_L; static std::string token0, token1;
        is >> idx >> token0 >> token1 >> reserve0_L >> reserve1_L;
        v2Pair[address] = new v2::V2Pair(   token0,
                                            token1,
                                            *(double *)(&reserve0_L),
                                            *(double *)(&reserve1_L),
                                            idx   );
        graph::addV2Pair(token0, token1, v2Pair[address]);
    }




    std::cout << "Recovered " << v3PoolAddress.size() << " v3 Pools and " << v2PairAddress.size() << " v2 Pairs." << std::endl;

    // v2pairs

    return LastIdx;
}

void SyncWithDb(unsigned long long lastBlockNumber){
    redis.set("UpdatedToBlockNumber", std::to_string(lastBlockNumber));

    // v3Pool
    for(auto u : lazyUpdatev3Pool) {
        auto v = v3Pool[u];
        size_t size = CopyPool(v->IntPool, (Pool<false> *)v3::buffer);
        redis.hset("v3poolsData", u, std::string(v3::buffer, size));
        redis.hset("v3poolsInfo", u, std::to_string(v->latestIdxHash) + " " + v->token[0] + " " + v->token[1] + " " + std::to_string(v->initialized));
    }

    // v2Pair
    for(auto u : lazyUpdatev2Pair) {
        auto v = v2Pair[u];
        unsigned long long reserve0_L = *(unsigned long long *)(&v->reserve[0]);
        unsigned long long reserve1_L = *(unsigned long long *)(&v->reserve[1]);
        redis.hset("v2pairsData", u, std::to_string(v->latestIdxHash) + " " + v->token[0] + " " + v->token[1] + " " + std::to_string(reserve0_L) + " " + std::to_string(reserve1_L));
    }


    try {
        redis.bgsave(); // 后台写入外存
    } catch(const Error & e) {
        std::cerr << "Background saving may fail, message: " << e.what() << std::endl;
    }
    std::cerr << "[S]   sync data with db v3 Pool cnt = " << lazyUpdatev3Pool.size() << " v2 pair cnt = " << lazyUpdatev2Pair.size() << std::endl;
}

int main(){
    initializeTicksPrice();
    std::ofstream fout("circle_founded_info.log");

    redis.del("queue");
    unsigned long long lastIdxHash = recoverStateFromDb();

    while("💤Shu💝Yu💖Mo💤") {
        auto result = redis.blpop("queue", 1); // 如果 `queue` 为空，阻塞 1 秒.
        if(!result){
            static int waitedtime = 0;
            std::cout << "[W]   The Next events after " << lastIdxHash << " have not been created yet. waitedtime = " << ++waitedtime << std::endl;
            continue;
        } else {
            // std::cout << "Got Events " << result->second << std::endl;
        }
        std::istringstream is(result->second);
        static v2::V2Event v2e;
        static v3::V3Event v3e;
        int vr;
        if(result->second[0] == '2') {
            vr = 2;
            v2e = v2::rawdata2event(is);
            lazyUpdatev2Pair.insert(v2e.address);
        } else {
            vr = 3;
            v3e = v3::rawdata2event(is);
            lazyUpdatev3Pool.insert(v3e.address);
        }


        auto nowBlockNumber = (vr == 2 ? v2e.idxHash : v3e.idxHash) / LogIndexMask;
        auto lastBlockNumber = lastIdxHash / LogIndexMask;
        if(lastBlockNumber != nowBlockNumber && lastBlockNumber % 1000 == 0) {
            // 块数编号是 100 的倍数 且 和上次处理的块号不同，同步至数据库。
            SyncWithDb(lastBlockNumber);
            lazyUpdatev2Pair.clear();
            lazyUpdatev3Pool.clear();
        }
        lastIdxHash = (vr == 2 ? v2e.idxHash : v3e.idxHash);

        if(vr == 3) { // V3 Pool
            if(v3e.type == v3::CRET){
                assert(!v3Pool.count(v3e.address));
                v3Pool[v3e.address] = new v3::V3Pool(   v3e.fee,
                                                        v3e.tickspace,
                                                        v3e.liquidity,
                                                        v3e.token0,
                                                        v3e.token1,
                                                        v3e.idxHash   );
                graph::addV3Pool(v3e.token0, v3e.token1, v3Pool[v3e.address]);
                std::cout << "[S]   New v3 pool " << v3e.address << " created and been listened. the number of recognised token = " << graph::token_num << " the number of pools = " << graph::v3_pool_num  << "." << std::endl;
            } else {
                assert(v3Pool.count(v3e.address));
                v3Pool[v3e.address]->processEvent(v3e);
            }
        } else { // V2 Pool
            if(v2e.type == v2::CRET) {
                if(v2Pair.count(v2e.address)) {
                    auto v2p = v2Pair[v2e.address];
                    assert(v2p->token[0] == v2e.token0 && v2p->token[1] == v2e.token1);
                } else {
                    v2Pair[v2e.address] = new v2::V2Pair(   v2e.token0,
                                                            v2e.token1,
                                                            0, 0,
                                                            v2e.idxHash   );
                }
                graph::addV2Pair(v2e.token0, v2e.token1, v2Pair[v2e.address]);
                std::cout << "[S]   New v2 pair " << v2e.address << " created and been listened. the number of recognised token = " << graph::token_num << " the number of pairs = " << graph::v2_pair_num  << "." << std::endl;
            } else {
                assert(v2Pair.count(v2e.address));
                v2Pair[v2e.address]->processEvent(v2e);
            }
        }

        // auto [found, circle] = graph::FindCircle();
        // if(found) {
        //     fout << "After transaction: " << (vr == 2 ? v2e.idxHash : v3e.idxHash) << std::endl;
        //     fout << circle << std::endl;
        // }
    }
    return 0;
}