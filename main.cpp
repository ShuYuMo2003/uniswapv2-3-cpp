#include "include/graph.h"
#include <map>
#include <cstring>
#include <ctime>
#include <unistd.h>
#include <sw/redis++/redis++.h> // with `-lredis++ -lhiredis -pthread`

using namespace sw::redis;

const int LogIndexMask = 1e5;

Redis redis = Redis("tcp://127.0.0.1:6379");

std::map<std::string, v3::V3Pool*> v3Pool;

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

    // v3Pools
    std::cout << "Recovering v3 pools .. " << std::endl;
    v3Pool.clear();
    graph::clearEdges();
    std::vector<std::string> v3PoolAddress{};
    redis.hkeys("v3poolsData", std::back_inserter(v3PoolAddress));
    for(auto address : v3PoolAddress) {
        auto rawdata = redis.hget("v3poolsData", address);
        auto rawinfo = redis.hget("v3poolsInfo", address);
        assert(rawdata && rawinfo);
        std::cout << "Got v3 Pool " << address <<" info: " << *rawinfo << std::endl;

        std::istringstream is(*rawinfo);
        static unsigned long long idx; static std::string token0, token1; static bool __init;
        is >> idx >> token0 >> token1 >> __init;
        v3Pool[address] = new v3::V3Pool( (Pool<false> * )(*rawdata).c_str(), token0, token1, idx, __init);
        std::cout << "Initialized " << address << std::endl;
        graph::addV3Pool(token0, token1, v3Pool[address]);
    }
    std::cout << "Recovered " << v3PoolAddress.size() << " v3 Pools" << std::endl;

    // v2pairs

    return LastIdx;
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
        }
        std::istringstream is(result->second);
        auto e = v3::rawdata2event(is);

        auto nowBlockNumber = e.idxHash / LogIndexMask;
        auto lastBlockNumber = lastIdxHash / LogIndexMask;
        if(lastBlockNumber != nowBlockNumber && lastBlockNumber % 1000 == 0) { // 块数编号是 100 的倍数 且 和上次处理的块号不同，同步至数据库。
            redis.set("UpdatedToBlockNumber", std::to_string(lastBlockNumber));
            for(auto [u, v] : v3Pool) {
                size_t size = CopyPool(v->IntPool, (Pool<false> *)v3::buffer);
                redis.hset("v3poolsData", u, std::string(v3::buffer, size));
                redis.hset("v3poolsInfo", u, std::to_string(v->latestIdxHash) + " " + v->token[0] + " " + v->token[1] + " " + std::to_string(v->initialized));
            }
            try {
                redis.bgsave(); // 后台写入外存
            } catch(const Error & e){
                std::cerr << "Background saving may fail, message: " << e.what() << std::endl;
            }
            std::cerr << "[S]   sync data with db v3 Pool cnt = " << v3Pool.size() << std::endl;
        }
        lastIdxHash = e.idxHash;

        if(e.type == v3::CRET){
            assert(!v3Pool.count(e.address));
            v3Pool[e.address] = new v3::V3Pool( e.fee,
                                                e.tickspace,
                                                e.liquidity,
                                                e.token0,
                                                e.token1,
                                                e.idxHash );
            graph::addV3Pool(e.token0, e.token1, v3Pool[e.address]);
            std::cout << "[S]   New v3 pool " << e.address << " created and been listened. the number of recognised token = " << graph::token_num << " the number of pools = " << graph::v3_pool_num  << "." << std::endl;
        } else {
            assert(v3Pool.count(e.address));
            v3Pool[e.address]->processEvent(e);
        }
        auto [found, circle] = graph::FindCircle();
        if(found) {
            fout << "After transaction: " << e.idxHash << std::endl;
            fout << circle << std::endl;
        }
    }
    return 0;
}