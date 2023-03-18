#ifndef headerfilegraph
#define headerfilegraph

#include <vector>
#include <map>
#include "v3pool.h"
#include "v2pair.h"
#include <algorithm>

namespace graph{



int token_num = 0, v2_pair_num = 0, v3_pool_num = 0;
std::map<std::string, uint> token2idx;
std::map<uint, std::string> idx2token;



enum PairType { UniswapV2, UniswapV3 };

struct Edge{
    PairType pair_type;
    void * aim;
    uint u, v, zeroToOne;
    double swap(double amountIn) const {
        if(pair_type == UniswapV3) return ((v3::V3Pool *)aim)->query(zeroToOne, amountIn);
        if(pair_type == UniswapV2) return ((v2::V2Pair *)aim)->query(zeroToOne, amountIn);
    }
};

std::vector< std::vector<Edge> > E;

struct GetCircleRes {
    double amount_in, st, revenue;
    std::vector<Edge> edges;
    friend std::ostream& operator<<(std::ostream &os, const GetCircleRes &res) {
        os << std::setiosflags(std::ios::fixed) << std::setprecision(20);
        os << "========== circle found ==========" << std::endl;
        os << res.amount_in << " " << " " << res.revenue << std::endl;
        for (auto edge : res.edges) {
            os << idx2token[edge.u] << " " << idx2token[edge.v] << std::endl;
        }
        os << "==================================" << std::endl;
        return os;
    }
};


void clearEdges() {
    token2idx.clear();
    idx2token.clear();
    E.clear();
    token_num = 0;
    v2_pair_num = 0;
    v3_pool_num = 0;
}


void addV3Pool(std::string token0, std::string token1, v3::V3Pool * pool) {
    if(!token2idx.count(token0)) {
        idx2token[token_num] = token0;
        token2idx[token0] = token_num++;
    }
    if(!token2idx.count(token1)) {
        idx2token[token_num] = token1;
        token2idx[token1] = token_num++;
    }

    auto u = token2idx[token0], v = token2idx[token1];
    E.resize(token_num);

    E[u].push_back({UniswapV3, pool, u, v, 1});
    E[v].push_back({UniswapV3, pool, v, u, 0});

    v3_pool_num++;
}

void addV2Pair(std::string token0, std::string token1, v2::V2Pair * pair) {
    if(!token2idx.count(token0)) {
        idx2token[token_num] = token0;
        token2idx[token0] = token_num++;
    }
    if(!token2idx.count(token1)) {
        idx2token[token_num] = token1;
        token2idx[token1] = token_num++;
    }

    auto u = token2idx[token0], v = token2idx[token1];
    E.resize(token_num);

    E[u].push_back({UniswapV2, pair, u, v, 1});
    E[v].push_back({UniswapV2, pair, v, u, 0});

    v2_pair_num++;
}

std::pair<bool, GetCircleRes> get_circle(int start_point, double init_amount) {
    static std::vector<double> d;
    static std::vector<std::pair<int, int>> st;
    static std::vector<int> vis;
    static int execute_count = 0;
    d.resize(token_num), st.resize(token_num), vis.resize(token_num, 0);
    execute_count++;
    int t = 0;
    for (int i = 0; i < token_num; ++i) d[i] = vis[i] = 0;
    st[t++] = {start_point, 0}, vis[start_point] = execute_count, d[start_point] = init_amount;
    GetCircleRes res; res.st = start_point;
    while (t) {
        auto [u, idx] = st[t - 1];
        // std::cerr << "\nGot " << u << std::endl;
        bool suc = false;
        for (uint i = idx; i < E[u].size(); ++i) {
            const Edge &edge = E[u][i];
            int v = edge.v;
            // std::cerr << "extend to " << v << std::endl;
            double nv = edge.swap(d[u]);
            // std::cerr << "Nv = " << nv << " d[v] = " << d[v] << std::endl;
            if (d[v] < nv) {
                st[t - 1].second = i + 1;
                if (vis[v] == execute_count) {
                    res.amount_in = init_amount;
                    while (st[--t].first != v) {
                        auto [k, i] = st[t];
                        res.edges.push_back(E[k][i - 1]);
                    }
                    assert(v == st[t].first);
                    res.edges.push_back(E[v][st[t].second - 1]);
                    res.revenue = nv - d[v];
                    std::reverse(res.edges.begin(), res.edges.end());
                    return std::make_pair(true, res);
                }
                d[v] = nv;
                vis[v] = execute_count;
                st[t++] = {v, 0};
                suc = true;
                break;
            }
        }
        if (!suc) {
            t--, vis[u] = 0;
        }
    }
    return std::make_pair(false, res);
}

std::pair<bool, GetCircleRes> FindCircle() {
    // std::cerr << "Finding Circle" << std::endl;
    GetCircleRes result;
    static std::string WETH_ADDRESS = "0xC02aaA39b223FE8D0A0e5C4F27eAD9083C756Cc2";
    if(!token2idx.count(WETH_ADDRESS))
        return (std::cerr << "NOT FOUND WETH TOKEN" << std::endl), std::make_pair(false, result);

    static double lim_amount = 1099511627776llu;
    double maxRevenue = -1;
    for (double init_amount = 10000; init_amount <= lim_amount; init_amount *= 1.5) {
        int st = token2idx[WETH_ADDRESS];
        // std::cerr << init_amount << " " << st << std::endl;
        auto [suc, res] = get_circle(st, init_amount);
        if(suc && res.revenue > maxRevenue) {
            maxRevenue = res.revenue;
            result = res;
            // std::cerr << "FOUND !! " << res.revenue << std::endl;
        }
    }
    return std::make_pair(maxRevenue > 0, result);
}





















}


#endif