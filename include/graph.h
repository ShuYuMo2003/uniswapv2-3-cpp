#ifndef headerfilegraph
#define headerfilegraph

#include <vector>
#include <unordered_map>
#include <optional>
#include "v3pool.h"
#include "v2pair.h"
#include "logger.h"
#include <algorithm>
#include <mutex>
#include <thread>
#include <random>
#include <queue>
#include <functional>
#include <unistd.h>

const std::string WETH_ADDRESS = "0xC02aaA39b223FE8D0A0e5C4F27eAD9083C756Cc2";
std::set<std::string> blackList{
    "0xd233D1f6FD11640081aBB8db125f722b5dc729dc",
    "0xA4C9b58D1Ce7EA0d9b351185E0c333683EbDe00b"
};

namespace graph{

typedef unsigned long long SwapVersion;

void evaluateTokens();

uint token_num = 0;
std::unordered_map<std::string, uint> token2idx;
std::unordered_map<uint, std::string> idx2token;

uint checkToken(const std::string & token) {
    if(!token2idx.count(token)) {
        idx2token[token_num] = token;
        token2idx[token] = token_num++;
        return token_num - 1;
    } else
        return token2idx[token];
}

enum PairType { UniswapV2, UniswapV3 };

struct Edge{
    PairType pair_type;
    void * aim;
    uint zeroToOne;
    Edge(const PairType & _pt, void * _aim, const uint & _zeroToOne) :
        pair_type(_pt), aim(_aim), zeroToOne(_zeroToOne)  {  }

    std::pair<double, SwapVersion> swap(double amountIn) const {
        if(pair_type == UniswapV3) return std::make_pair( ((v3::V3Pool *)aim)->query(zeroToOne, amountIn), ((v3::V3Pool *)aim)->latestIdxHash);
        if(pair_type == UniswapV2) return std::make_pair( ((v2::V2Pair *)aim)->query(zeroToOne, amountIn), ((v2::V2Pair *)aim)->latestIdxHash);
    }
};


class EdgeGroup{
public:
    std::vector<Edge> es{};
    std::unordered_map<uint, std::string> Idx2Address{};
    uint u, v;
    EdgeGroup(uint _u, uint _v) : u(_u), v(_v) {}
    void add(PairType pair_type, const std::string address, bool zeroToOne, void * o) {
        Idx2Address[es.size()] = address;
        es.emplace_back(Edge(pair_type, o, zeroToOne));
    }
    std::optional<std::tuple<double, uint, SwapVersion>> query(double amount) const {
        double result = -1; uint midx = -1;
        SwapVersion mver;
        for(uint idx = 0; idx < es.size(); idx++){
            auto currentResult = es[idx].swap(amount);
            if(result < currentResult.first){
                result = currentResult.first;
                midx = idx;
                mver = currentResult.second;
            }
        }
        if(result < 1) return {};
        else return std::make_tuple(result, midx, mver);
    }
};

struct puuHash{
    size_t operator() (const std::pair<uint, uint> & k) const {
        return (k.first << 15u) ^ k.second;
    }
};
struct puuEqual{
    bool operator() (const std::pair<uint, uint> & lhs, const std::pair<uint, uint> & rhs) const {
        return lhs.first == rhs.first && lhs.second == rhs.second;
    }
};
typedef std::vector< std::vector<EdgeGroup*> > EdgeSet;

EdgeSet E; std::mutex oGraphBlock;
std::unordered_map<std::pair<uint, uint> , EdgeGroup *, puuHash, puuEqual> EM;

std::vector<bool> available;

void addEdge(const std::string & token0, const std::string & token1, PairType type, void * o, const std::string & address, bool reEvaluate = false) {
    oGraphBlock.lock();
    // std::cerr << "Adding " << address << " (" << token0 << ", " << token1 << ") "  << type << " ";
    static int DelayEvaluate = 0;
    uint u = checkToken(token0);
    uint v = checkToken(token1);
    if(E.size() < token_num)
        E.resize(token_num);
    // std::cerr << u << " " << v << std::endl;
    bool newActualEdge = false;
    if(!EM.count(std::make_pair(u, v))) {
        EM[std::make_pair(u, v)] = new EdgeGroup(u, v);
        EM[std::make_pair(v, u)] = new EdgeGroup(v, u);
        E[u].push_back(EM[std::make_pair(u, v)]);
        E[v].push_back(EM[std::make_pair(v, u)]);
        if(reEvaluate)
            Logger(std::cout, INFO, "addEdge") << "not found edge(" << token0 << " -> " << token1 << "). build new edge." << kkl();
        newActualEdge = true;
    } else {

    }
    EM[std::make_pair(u, v)]->add(type, address, 1, o);
    EM[std::make_pair(v, u)]->add(type, address, 0, o);

    if(reEvaluate and newActualEdge) {
        DelayEvaluate++;
        if(DelayEvaluate % 10 == 0) // do not re evaluate tokens immedriatly.
            evaluateTokens();
        else
            available.resize(token_num);
    }
    oGraphBlock.unlock();
}

void clearEdges() {
    token2idx.clear();
    idx2token.clear();
    E.clear();
    token_num = 0;
}

struct CircleInfoTaker_t{
    double amountIn, revenue;
    std::vector<std::tuple<std::string, bool, SwapVersion>> plan;
    void add(EdgeGroup * now, uint idx, SwapVersion vv) {
        plan.emplace_back(std::make_tuple(now->Idx2Address[idx], now->es[idx].zeroToOne, vv));
    }
    void clear() { plan.clear(); amountIn = 0; revenue = 0; }
    bool operator < (const CircleInfoTaker_t & rhs) const  { return revenue < rhs.revenue; }
};


std::priority_queue<CircleInfoTaker_t> producePlan; std::mutex ppBlock;


namespace Tarjan{ // Great Tarjan !

std::vector<int> dfn;
std::vector<int> low;
std::vector<int> s;
int idx, bcc, top;
std::vector<std::vector<uint>> bccSet;

inline void tarjan(int u, int fa) {
    int son = 0;
    low[u] = dfn[u] = ++idx;
    s[++top] = u;
    for(auto G : E[u]) {
        uint v = G->v;
        if(!dfn[v]) {
            son++;
            tarjan(v, u);
            low[u] = std::min(low[u], low[v]);
            if(low[v] >= dfn[u]) {
                bccSet.resize(++bcc);
                while(s[top + 1] != v) bccSet[bcc - 1].push_back(s[top--]);
                bccSet[bcc - 1].push_back(u);
            }
        } else if(v != fa) low[u] = std::min(low[u], dfn[v]);
    }
    if(fa == 0 && son == 0) {
        bccSet.resize(++bcc);
        bccSet[bcc - 1].push_back(u);
    }
}

void main(){
    idx = 0; top = 0; bcc = 0;
    dfn.resize(token_num + 5); for(auto & u : dfn) u = 0;
    low.resize(token_num + 5); for(auto & u : low) u = 0;
    s.resize(token_num + 5);  for(auto & u : s) u = 0;
    bccSet.resize(0);
    for(int i = 0; i < token_num; i++){
        if(!dfn[i]) {
            top = 0;
            tarjan(i, 0);
        }
    }
}
}

class core_t{
public:
    const double MINREVENUE = 1e17;
    const uint MAX_CALL_TIME = 0.5e4;

    std::vector<double> d;
    std::vector<uint> inStack;
    uint InStackMask;
    uint aim;
    CircleInfoTaker_t taker;
    uint call_cnt = 0;

    bool dfs(uint now, const EdgeSet & E) {
        // std::cerr << "\nnow at " << now << " d = " << d[now] << std::endl;
        if(++call_cnt > MAX_CALL_TIME)
            return false;
        for(auto group : E[now]) {
            if(inStack[group->v] == InStackMask)
                continue;


            auto result = group->query(d[now]);
            if(!result)
                continue;

            uint v = group->v;
            auto [nd, idx, version] = *result;
            // std::cerr << "now = " << now  << " with " << d[now] << " Transfer to = " << v << " nd = " << nd << std::endl;
            if(nd > d[v]) {
                if(v == aim) {
                    if(nd - d[v] > MINREVENUE) {
                        taker.revenue = nd - d[v];
                        taker.add(group, idx, version);
                        return true;
                    } else {
                        continue;
                    }
                } else {
                    d[v] = nd;
                    inStack[v] = InStackMask;
                    if(dfs(v, E)) {
                        taker.add(group, idx, version);
                        return true;
                    }
                    inStack[v] = InStackMask - 1;
                }
                if(call_cnt > MAX_CALL_TIME)
                    return false;
            }
        }
        return false;
    }

    std::optional<CircleInfoTaker_t> operator()(double amountIn, const EdgeSet & E) {

        d.resize(token_num);  for(auto & u : d) u = 0;

        static uint execute_cnt = 0;
        inStack.resize(token_num);
        InStackMask = ++execute_cnt;

        aim = token2idx[WETH_ADDRESS];
        d[aim] = amountIn;
        inStack[aim] = InStackMask - 1;
        taker.clear();
        taker.amountIn = amountIn;


        call_cnt = 0;
        bool result = dfs(aim, E);
        if(result) return taker;
        else return {};
    }

};

int availableVersion;
void evaluateTokens() {

    // Logger(std::cout, INFO, "evaluateTokens") << "avBlock locked" << kkl(); ////
    Logger(std::cout, INFO, "evaluateTokens") << "evaluating Tokens" << kkl();
    if(!token2idx.count(WETH_ADDRESS)){
        Logger(std::cout, ERROR, "evaluateTokens") << "Not found WETH token in graph." << kkl();
    }
    Tarjan::main();

    available.resize(token_num);
    for(int i = 0; i < token_num; i++) available[i] = 0;
    auto startPtr = token2idx[WETH_ADDRESS];
    for(auto & U : Tarjan::bccSet) {
        if(U.size() <= 2u){
            continue;
        }
        bool exist = false;
        for(auto & V : U) {
            if(V == startPtr) {
                exist = true;
                break;
            }
        }
        if(exist){
            for(auto & V : U) available[V] = 1;
        }
    }
    for(auto & garbage : blackList) {
        available[token2idx[garbage]] = false;
    }
    int cnt = 0; for(int i = 0; i < static_cast<int>(available.size()); i++) cnt += available[i];
    Logger(std::cout, INFO, "evaluateTokens") << "re-evaluate Tokens done. avaliable tokens = " << cnt << kkl();
    // Logger(std::cout, INFO, "evaluateTokens debug") << "avBlock ununlocked" << kkl();////
    ++availableVersion;
}

namespace mutithread{



void handle(int threadId) {
    static const double MIN = 1e17;
    static const double MAX = 1e19;

    int localAvVersion = -1;
    core_t core;
    EdgeSet PE;
    std::random_device rd;
    std::mt19937 g(rd() + threadId * 20031006u);

    while("💤Shu💝Yu💖Mo💤") {
        if(localAvVersion != availableVersion) {
            oGraphBlock.lock();
            Logger(std::cout, INFO, "SPFA " + std::to_string(threadId)) << "available token list updated detacted. rebuilding graph.." << kkl();
            PE.resize(token_num);
            for(int i = 0; i < token_num; i++) {
                PE[i].clear();
                for(auto & aim : E[i]) {
                    if(available[i] && available[aim->v]) {
                        PE[i].push_back(aim);
                    }
                }
            }
            localAvVersion = availableVersion;
            Logger(std::cout, INFO, "SPFA " + std::to_string(threadId)) << "build graph done" << kkl();
            oGraphBlock.unlock();
        }


        // Logger(std::cerr, ERROR, "SPFA " + std::to_string(threadId) + " debug") << PE.size() << kkl();////
        for(int i = 0; i < static_cast<int>(PE.size()); i++)
            shuffle(PE[i].begin(), PE[i].end(), g);

        // int aim = token2idx[WETH_ADDRESS];
        // for(int i = 0; i < token_num; i++) {
        //     int id = -1;
        //     for(int j = 0; j < static_cast<int>(PE[i].size()); j++) {
        //         if(PE[i][j]->v == aim) {
        //             id = j;
        //             break;
        //         }
        //     }
        //     if(id != -1) std::swap(PE[i][0], PE[i][id]);
        // }

        for(double amount = MIN; amount <= MAX; amount *= 2.2) {
            auto tempResult = core(amount, PE);
            if(tempResult) {
                ppBlock.lock();
                // Logger(std::cout, INFO, "SPFA " + std::to_string(threadId)) << "Found Plan: init_amount = " << tempResult->amountIn / (1e18) << "eth revenue = "
                //         << tempResult->revenue / (1e18) << "eth stepCnt = " << tempResult->plan.size() << " queueSize = " << producePlan.size() << kkl();
                producePlan.push(*tempResult);
                ppBlock.unlock();
            }
        }
    }


}
}

namespace handlePlans{
    void main(std::function<void (CircleInfoTaker_t)> callback){
        // listerning producePlan.
        Logger(std::cout, INFO, "handlePlans") << "Listerning Plans Maker." << kkl();
        while("💤Shu💝Yu💖Mo💤") {
            // Logger(std::cout, INFO, "handlePlans") << "Listerning Plans Maker. producePlan.size() = " << producePlan.size() << kkl();
            if(producePlan.size() > 0u) {
                ppBlock.lock();
                CircleInfoTaker_t ans = producePlan.top();
                // while(producePlan.size())
                producePlan.pop();
                // Logger(std::cout, IMPO, "handlePlans") << "Received a plan. revenue = " << ans.revenue / (1e18) << "eth" << kkl();
                ppBlock.unlock();
                callback(ans);
            }

            usleep(0.5/*ms*/ * 1000);
        }
    }
}

std::vector<std::thread> threads;
void BuildThreads(std::function<void (CircleInfoTaker_t)> callback) {
    evaluateTokens();
    assert(THREAD_COUNT >= 2);
    for(int i = 0; i < THREAD_COUNT - 1; i++) {
        std::thread th(mutithread::handle, i);
        threads.push_back(std::move(th));
    }
    Logger(std::cout, INFO, "BuildThreads") << "SPFA sub-thread built done. SPFA count = " << THREAD_COUNT - 1 << kkl();

    std::thread planThread(handlePlans::main, callback);
    threads.push_back(std::move(planThread));

    Logger(std::cout, INFO, "BuildThreads") << "Plan Resolve sub-thread built done." << kkl();

    for(auto & thread : threads) {
        thread.detach();
    }

    Logger(std::cout, INFO, "BuildThreads") << "All Threads detached." << kkl();
}



}


#endif