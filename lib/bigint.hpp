#include<utility>
#include<algorithm>
typedef unsigned int uint;
typedef long long ll;
typedef unsigned long long ull;
ull ones32 = (1ULL<<32) - 1;
// template<std::uint digits_num>
// class bigint
// {
//     static storage_len = (digits_num - 1) / 32 + 1;
//     uint *_v;
// public:
//     bigint(ll v = 0) {
//         v = new ull[storage_len];
//         for (int i = 0; i < storage_len - 1; ++i) {
//             digits[i] += v & ones32;
//             digits[i + 1] += v >> 32;
//         }
//         digits[storage_len - 1] += v & ones32;
//     }
//     ~bigint() { delete v[]; }
//     friend bigint operator+(const int256 &a, const int256 &b) {
//         bigint res;
//         for (int i = 0; i < 8; ++i) {
            
//         }
//     }
// };
template<uint value_size>
class UInt
{
public:
    uint table[value_size];
    uint size() const {
		return value_size;
	}
    biguint(uint storage_len, ull initial = 0) : storage_len(storage_len) {
        v = new ull[storage_len];
        for (int i = 0; i < storage_len - 1; ++i) {
            v[i] += initial & ones32;
            v[i + 1] += v[i] >> 32;
            v[i] &= ones32;
            initial >>= 32;
        }
        v[storage_len - 1] += initial & ones32;
    }
    biguint(const biguint &a) {
        if (v) delete[] v;
        v = new ull[storage_len];
        for (int i = 0; i < std::min(storage_len, a.storage_len); ++i) {
            v[i] = a.v[i];
        }
    }
    ~biguint() { delete[] v; }
    friend bool operator<(const biguint &a, const biguint &b) {
        uint res_len = std::max(a.storage_len, b.storage_len);
        for (int i = res_len - 1; ~i; ++i) {
            ull v1 = 0, v2 = 0;
            if (i < a.storage_len) v1 = a.v[i];
            if (i < b.storage_len) v2 = b.v[i];
            if (v1 < v2) return true;
            if (v2 > v1) return false;
        }
        return false;
    }
    friend bool operator>(const biguint &a, const biguint &b) {
        return b < a;
    }
    friend bool operator<=(const biguint &a, const biguint &b) {
        uint res_len = std::max(a.storage_len, b.storage_len);
        for (int i = res_len - 1; ~i; ++i) {
            ull v1 = 0, v2 = 0;
            if (i < a.storage_len) v1 = a.v[i];
            if (i < b.storage_len) v2 = b.v[i];
            if (v1 < v2) return true;
            if (v2 > v1) return false;
        }
        return true;
    }
    friend bool operator>=(const biguint &a, const biguint &b) {
        return b <= a;
    }
    friend bool operator==(const biguint &a, const biguint &b) {
        uint res_len = std::max(a.storage_len, b.storage_len);
        for (int i = res_len - 1; ~i; ++i) {
            ull v1 = 0, v2 = 0;
            if (i < a.storage_len) v1 = a.v[i];
            if (i < b.storage_len) v2 = b.v[i];
            if (v1^v2) return false;
        }
        return true;
    }
    friend biguint max(const biguint &a, const biguint &b) {
        
        uint res_len = std::max(a.storage_len, b.storage_len);
        for (int i = res_len - 1; ~i; ++i) {
            ull v1 = 0, v2 = 0;
            if (i < a.storage_len) v1 = a.v[i];
            if (i < b.storage_len) v2 = b.v[i];
            if (v1 < v2) return true;
            if (v2 > v1) return false;
        }
        return false;
    }
    friend biguint operator+(const biguint &a, const biguint &b) {
        uint res_len = std::max(a.storage_len, b.storage_len);
        biguint res(res_len);
        for (int i = 0; i < res_len; ++i) {
            ull v1 = 0, v2 = 0;
            if (i < a.storage_len) v1 = a.v[i];
            if (i < b.storage_len) v2 = b.v[i];
            res.v[i] = v1 + v2;
            if (i + 1 < res_len) res.v[i + 1] += res.v[i] >> 32;
            res.v[i] &= ones32;
        }
        return res;
    }
    friend biguint operator-(const biguint &a, const biguint &b) {
        const biguint &c = a, &d = b;
        if (a < b) 
        uint res_len = std::max(a.storage_len, b.storage_len);
        biguint res(res_len);
        for (int i = 0; i < res_len; ++i) {
            ull v1 = 0, v2 = 0;
            if (i < a.storage_len) v1 = a.v[i];
            if (i < b.storage_len) v2 = b.v[i];
            res.v[i] = v1 + v2;
            if (i + 1 < res_len) res.v[i + 1] += res.v[i] >> 32;
            res.v[i] &= ones32;
        }
        return res;
    }
};