#ifndef headerfilegraph
#define headerfilegraph

#include <vector>
#include <unordered_map>
#include <optional>
#include "v3pool.h"
#include "v2pair.h"
#include <algorithm>
#include <mutex>
#include <thread>

const std::string WETH_ADDRESS = "0xC02aaA39b223FE8D0A0e5C4F27eAD9083C756Cc2";
namespace graph{

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

    double swap(double amountIn) const {
        if(pair_type == UniswapV3) return ((v3::V3Pool *)aim)->query(zeroToOne, amountIn);
        if(pair_type == UniswapV2) return ((v2::V2Pair *)aim)->query(zeroToOne, amountIn);
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
    std::optional<std::pair<double, uint>> query(double amount) const {
        double result = -1; uint midx = -1;
        for(uint idx = 0; idx < es.size(); idx++){
            if(result < es[idx].swap(amount)){
                result = es[idx].swap(amount);
                midx = idx;
            }
        }
        if(result < 1) return {};
        else return std::make_pair(result, midx);
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
EdgeSet E;
std::unordered_map<std::pair<uint, uint> , EdgeGroup *, puuHash, puuEqual> EM;


void addEdge(const std::string & token0, const std::string & token1, PairType type, void * o, const std::string & address) {
    // std::cerr << "Adding " << address << " (" << token0 << ", " << token1 << ") "  << type << " ";

    uint u = checkToken(token0);
    uint v = checkToken(token1);
    if(E.size() < token_num)
        E.resize(token_num);
    // std::cerr << u << " " << v << std::endl;
    if(!EM.count(std::make_pair(u, v))) {
        EM[std::make_pair(u, v)] = new EdgeGroup(u, v);
        EM[std::make_pair(v, u)] = new EdgeGroup(v, u);
        E[u].push_back(EM[std::make_pair(u, v)]);
        E[v].push_back(EM[std::make_pair(v, u)]);
    } else {

    }
    EM[std::make_pair(u, v)]->add(type, address, 1, o);
    EM[std::make_pair(v, u)]->add(type, address, 0, o);

}

void clearEdges() {
    token2idx.clear();
    idx2token.clear();
    E.clear();
    token_num = 0;
}

struct CircleInfoTaker_t{
    double amountIn, revenue;
    std::vector<std::pair<std::string, bool>> plan;
    void add(EdgeGroup * now, uint idx) {
        plan.emplace_back(std::make_pair(now->Idx2Address[idx], now->es[idx].zeroToOne));
    }
    void clear() { plan.clear(); amountIn = 0; revenue = 0; }
};

std::vector<bool> available;

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

namespace core{
const double MINREVENUE = 1e17;
const uint MAX_CALL_TIME = 1e4;

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
        if(inStack[group->v] == InStackMask || (!available[group->v]))
            continue;

        auto result = group->query(d[now]);
        if(!result)
            continue;

        uint v = group->v;
        auto [nd, idx] = *result;
        // std::cerr << "now = " << now  << " with " << d[now] << " Transfer to = " << v << " nd = " << nd << std::endl;
        if(nd > d[v]) {
            if(v == aim) {
                if(nd - d[v] > MINREVENUE) {
                    taker.revenue = nd - d[v];
                    taker.add(group, idx);
                    return true;
                } else {
                    continue;
                }
            } else {
                d[v] = nd;
                inStack[v] = InStackMask;
                if(dfs(v, E)) {
                    taker.add(group, idx);
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

std::optional<CircleInfoTaker_t> main(double amountIn, const EdgeSet & E){

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

}

void evaluateTokens() {
    if(!token2idx.count(WETH_ADDRESS)){
        std::cerr << "NOT FOUND WETH TOKEN" << std::endl;
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
}

namespace mutithread{

double MaxRevenue;
CircleInfoTaker_t result;

std::mutex Blocker;

void handle() {
    static const double MIN = 1e17;
    static const double MAX = 1e19;

    EdgeSet PE; PE.resize(token_num);
    for(int i = 0; i < token_num; i++) {
        for(auto aim : E[i]) {
            if(available[i] && available[aim->v]) {
                PE[i].push_back(aim);
            }
        }
        random_shuffle(PE[i].begin(), PE[i].end());
    }
    std::cerr << "random done" << std::endl;
    std::cerr << std::this_thread::get_id() << " handler ready start to run" << std::endl;
    CircleInfoTaker_t tempResult;
    for(double amount = MIN; amount <= MAX; amount *= 2.2) {
        auto temp = core::main(amount, PE);
        if(temp) {
            tempResult = *temp;
        }
    }
    std::cerr << std::this_thread::get_id() << " run done. " << std::endl;

    Blocker.lock();
    if(MaxRevenue < tempResult.revenue) {
        MaxRevenue = tempResult.revenue;
        result = tempResult;
    }
    Blocker.unlock();
}
}



std::optional<CircleInfoTaker_t> findCircle(){
    const int THREAD_COUNT = 1;
    if(available.size() < token_num)
        evaluateTokens();

    mutithread::MaxRevenue = -1;

    std::vector<std::thread> threads;
    for(int i = 0; i < THREAD_COUNT; i++) {
        std::thread th(mutithread::handle);
        threads.push_back(std::move(th));
    }
    for(auto & thread : threads) {
        thread.join();
    }
    if(mutithread::MaxRevenue > 0) {
        return mutithread::result;
    } else {
        return {};
    }
}
}


#endif