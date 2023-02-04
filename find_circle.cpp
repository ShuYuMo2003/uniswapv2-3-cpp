#include <vector>

#include "include/pool.h"

struct V3Pool {
    static long long swap_total_time;
    static int swap_cnt;
    Pool pool;
    friend std::istream& operator>>(std::istream &is, V3Pool &pool) {
        return is >> pool.pool;
    }
    uint256 swap(bool zeroToOne, int256 amountIn) {
        long long st = clock();
        auto [amount0, amount1] = pool.swap("0x0", zeroToOne, amountIn, zeroToOne ? "4295128740" : "1461446703485210103287273052203988822378723970341", "", false);
        swap_total_time += clock() - st, swap_cnt++;
        // ASSERT(amountIn == realIn, "amount");
        // std::cout << "???? " << amountIn << " " << amount0 << " " << amount1 << std::endl;
        return zeroToOne ? -amount1 : -amount0;
    }
};
long long V3Pool::swap_total_time = 0;
int V3Pool::swap_cnt = 0;

struct V2Pair {
    // int token0, token1;
    uint256 reserve0, reserve1;
    V2Pair() : reserve0(0), reserve1(0) {}
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
        return os << edge.pair_type << " " << edge.u << " " << edge.v << " " << edge.idx << " " << edge.zeroToOne;
    }
    uint256 swap(uint256 amountIn) const {
        uint256 res = -1;
        if (pair_type == UniswapV2) res = v2_pairs[idx].swap(zeroToOne, amountIn);
        if (pair_type == UniswapV3) res = v3_pools[idx].swap(zeroToOne, amountIn);
        return res;
    }
};

struct GetCircleRes {
    uint256 amount_in, st, revenue;
    std::vector<Edge> edges;
    friend std::ostream& operator<<(std::ostream &os, const GetCircleRes &res) {
        os << "========== circle found ==========" << std::endl;
        os << res.amount_in << " " << res.st << " " << res.revenue << std::endl;
        for (auto edge : res.edges) {
            os << edge << std::endl;
        }
        os << "==================================" << std::endl;
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
    if (!execute_count) d.resize(token_num), st.resize(token_num), vis.resize(token_num, 0);
    execute_count++;
    int t = 0;
    for (int i = 0; i < token_num; ++i) d[i] = vis[i] = 0;
    st[t++] = {start_point, 0}, vis[start_point] = execute_count, d[start_point] = init_amount;
    GetCircleRes res; res.st = start_point;
    while (t) {
        auto [u, idx] = st[t - 1];
        // std::cout << t << " " << u << " " << idx << " " << E[u].size() << std::endl;
        bool suc = false;
        for (int i = idx; i < E[u].size(); ++i) {
            const Edge &edge = E[u][i];
            // std::cout << i << " " << E[u].size() << std::endl;
            // std::cout << edge << std::endl;
            int v = edge.v;
            // puts("???");
            uint256 nv = edge.swap(d[u]);
            // std::cout << edge << std::endl;
            // std::cout << d[u] << " " << nv << " " << d[v] << std::endl;
            if (d[v] < nv) {
                // std::cout << u << " " << v << " " << d[v] << " " << nv << std::endl;
                st[t - 1].second = i + 1;
                // std::cout << vis[v] << " " << execute_count << std::endl;
                if (vis[v] == execute_count) {
                    // puts("!!!");
                    res.amount_in = init_amount;
                    // std::cout << st[t - 1].first << " " << st[t - 1].second << std::endl;
                    while (st[--t].first != v) {
                        auto [k, i] = st[t];
                        res.edges.push_back(E[k][i - 1]);
                        // std::cout << st[t - 1].first << " " << st[t - 1].second << std::endl;
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
            // std::cout << "out " << u << std::endl;
            t--, vis[u] = 0;
        }
    }
    return std::make_pair(false, res);
}

int main() {
    std::ios::sync_with_stdio(false);
    freopen("input_for_algo", "r", stdin);
    std::cin >> token_num >> v2_pair_num >> v3_pool_num;
    E.resize(token_num), v2_pairs.resize(v2_pair_num), v3_pools.resize(v3_pool_num);
    for (int i = 0; i < v2_pair_num; ++i) {
        std::cout << "Reading: " << (i + 1) << " v2pair.\n";
        int u, v;
        std::cin >> u >> v >> v2_pairs[i];
        E[u].push_back({UniswapV2, u, v, i, 1});
        E[v].push_back({UniswapV2, v, u, i, 0});
    }
    for (int i = 0; i < v3_pool_num; ++i) {
        std::cout << "Reading: " << (i + 1) << " v3pool.\n";
        int u, v;
        std::cin >> u >> v >> v3_pools[i].pool;
        E[u].push_back({UniswapV3, u, v, i, 1});
        E[v].push_back({UniswapV3, v, u, i, 0});
    }
    uint256 lim_amount = 1; for (int i = 0; i < 40; ++i) lim_amount *= 2;
    std::cout << std::setiosflags(std::ios::fixed) << std::setprecision(6);
    for (uint256 init_amount = 10000; init_amount <= lim_amount; init_amount = init_amount * 15 / 10) {
        // std::cout << init_amount << std::endl;
        int st = 0, lst = V3Pool::swap_cnt;
        long long st_time = clock();
        auto [suc, res] = get_circle(st, init_amount);
        std::cout << "Get circle used " << (1e3 * (clock() - st_time) / CLOCKS_PER_SEC) << " ms with " << (V3Pool::swap_cnt - lst) << " v3swap call." << std::endl;
        if (!suc) continue;
        std::cout << res;
        // break;
    }
    std::cout << "Average time of v3 swap: " << (1e9 * V3Pool::swap_total_time / V3Pool::swap_cnt / CLOCKS_PER_SEC) << " ns." << std::endl;
}