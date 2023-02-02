#include <vector>

#include "include/pool.h"

struct V3Pool {
    Pool pool;
    friend std::istream& operator>>(std::istream &is, V3Pool &pool) {
        return is >> pool.pool;
    }
    uint256 swap(bool zeroToOne, uint256 amountIn) {
        Pool back = pool;
        auto [realIn, amountOut] = pool.swap("0x0", zeroToOne, amountIn, zeroToOne ? "4295128740" : "1461446703485210103287273052203988822378723970341", "");
        // ASSERT(amountIn == realIn, "amount");
        pool = back;
        return amountOut;
    }
};

struct V2Pair {
    // int token0, token1;
    uint256 reserve0, reserve1;
    friend std::istream& operator>>(std::istream &is, V2Pair &pair) {
        return is >> pair.reserve0 >> pair.reserve1;
        // return is >> pair.token0 >> pair.token1 >> pair.reserve0 >> pair.reserve1;
    }
    uint256 swap(bool zeroToOne, uint256 amountIn) {
        amountIn = amountIn * 997;
        uint256 numerator, denominator;
        if (zeroToOne) {
            numerator = amountIn * reserve1;
            denominator = reserve0 * 1000 + amountIn;
        } else {
            numerator = amountIn * reserve0;
            denominator = reserve1 * 1000 + amountIn;
        }
        return numerator / denominator;
    }
};

enum PairType { UniswapV2, UniswapV3 };

std::vector<V2Pair> v2_pairs;
std::vector<V3Pool> v3_pools;

struct Edge {
    PairType pair_type;
    int u, v, idx, zeroToOne;
    friend std::ostream& operator<<(std::ostream &os, const Edge &edge) {
        return os << edge.pair_type << " " << edge.u << " " << edge.v << " " << edge.idx << " " << edge.zeroToOne << std::endl;
    }
    uint256 swap(uint256 amountIn) const {
        if (pair_type == UniswapV2) return v2_pairs[idx].swap(zeroToOne, amountIn);
        if (pair_type == UniswapV3) return v3_pools[idx].swap(zeroToOne, amountIn);
        return -1;
    }
};

struct GetCircleRes {
    uint256 amount_in, st, revenue;
    std::vector<Edge> edges;
    friend std::ostream& operator<<(std::ostream &os, const GetCircleRes &res) {
        os << res.amount_in << " " << res.st << " " << res.revenue << std::endl;
        for (auto edge : res.edges) {
            os << edge << std::endl;
        }
        return os;
    }
};

std::vector<std::vector<Edge>> E;
int token_num, v2_pair_num, v3_pool_num;

std::pair<bool, GetCircleRes> get_circle(int start_point, uint256 init_amount) {
    static std::vector<uint256> d;
    static std::vector<std::pair<int, int>> st;
    static std::vector<int> vis;
    static int execute_count = 0;
    if (!execute_count) d.resize(token_num), st.resize(token_num), vis.resize(token_num);
    execute_count++;
    int t = 0;
    for (int i = 0; i < token_num; ++i) d[i] = 0;
    st[t++] = {start_point, 0}, vis[start_point] = execute_count, d[start_point] = init_amount;
    GetCircleRes res;
    while (t) {
        auto [u, idx] = st[t - 1];
        for (int i = idx; i < E[u].size(); ++i) {
            const Edge &edge = E[u][i];
            int v = edge.v;
            uint256 nv = edge.swap(d[u]);
            if (d[v] < nv) {
                if (vis[v]) {
                    res.amount_in = init_amount;
                    while (st[--t].first != v) {
                        auto [k, i] = st[t];
                        res.edges.push_back(E[k][i - 1]);
                    }
                    res.edges.push_back(E[v][st[t - 1].second - 1]);
                    std::reverse(res.edges.begin(), res.edges.end());
                    return std::make_pair(true, res);
                }
                d[v] = nv;
                vis[v] = true;
                st[t - 1].second = i + 1;
                st[t++] = {v, 0};
                break;
            }
        }
        t--, vis[u] = 0;
    }
    return std::make_pair(false, res);
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin >> token_num >> v2_pair_num >> v3_pool_num;
    E.resize(token_num);
    for (int i = 0; i < v2_pair_num; ++i) {
        int u, v;
        std::cin >> u >> v >> v2_pairs[i];
        E[u].push_back({UniswapV2, u, v, i, 1});
        E[v].push_back({UniswapV2, v, u, i, 0});
    }
    for (int i = 0; i < v3_pool_num; ++i) {
        int u, v;
        std::cin >> u >> v >> v3_pools[i].pool;
        E[u].push_back({UniswapV3, u, v, i, 1});
        E[v].push_back({UniswapV3, v, u, i, 0});
    }
    uint256 lim_amount = 1; for (int i = 0; i < 20; ++i) lim_amount *= 2;
    for (uint256 init_amount = 10000; init_amount <= lim_amount; init_amount = init_amount * 15 / 10) {
        int st = 0;
        auto [suc, res] = get_circle(st, init_amount);
        if (!suc) continue;
        std::cout << res;
    }
}