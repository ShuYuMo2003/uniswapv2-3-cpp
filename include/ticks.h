#ifndef headerfiletickbitmap
#define headerfiletickbitmap

#include <map>

#include "types.h"
#include "util.h"
#include "_tick.h"
#include <set>
#include <vector>
#include <algorithm>


#define GWordPos(a) ((a) >> 8)
#define fetchLowerBound(a) ((a) << 8) // 右移负数是 UB
#define fetchUpperBound(a) (((a) << 8) | 255)

template<bool enable_float>
inline bool tickCmp(const _Tick<enable_float> & lhs, const _Tick<enable_float> & rhs) {
    return lhs.id < rhs.id;
}

template<bool enable_float>
struct Ticks{
    size_t length;
    _Tick<enable_float> temp;
    // 不包括 temp, temp 的下一个位置开始，长度为 length 的一段内存 存储_Tick 信息。
    Ticks() { length = 0; }
};

std::pair<int16, int16> position(int24 tick) {
    // TODO: check 负数左移？
    return std::make_pair(int16(tick >> 8), uint8(tick - (int16(tick >> 8) * 256)));
}

template<bool enable_float>
_Tick<enable_float> * fetchTick(Ticks<enable_float> * o, int24 tick) {
    _Tick<enable_float> * beginPtr = (&o->temp) + 1;
    _Tick<enable_float> * endPtr   = beginPtr + o->length;

    o->temp.id = tick;
    _Tick<enable_float> * aim = std::lower_bound(beginPtr, endPtr, o->temp, tickCmp<enable_float>);
    if(aim == endPtr || aim->id != tick){
        std::memmove(aim + 1, aim, sizeof(o->temp) * (endPtr - aim));
        o->length++;
        *aim = _Tick<enable_float>(tick);
    }
    return aim;
}

template<bool enable_float>
void clearTick(Ticks<enable_float> * o, int24 tick) { // 如果存在 则清空 否则不处理。
    _Tick<enable_float> * beginPtr = (&o->temp) + 1;
    _Tick<enable_float> * endPtr   = beginPtr + o->length;

    o->temp.id = tick;
    _Tick<enable_float> * aim = std::lower_bound(beginPtr, endPtr, o->temp, tickCmp<enable_float>);
    if(aim != endPtr && aim->id == tick){
        aim->liquidityGross = 0;
        aim->liquidityNet = 0;
    }
}

template<bool enable_float>
void eraseTick(Ticks<enable_float> * o, int24 tick, int24 tickspace) {

    _Tick<enable_float> * beginPtr = (&o->temp) + 1;
    _Tick<enable_float> * endPtr   = beginPtr + o->length;

    o->temp.id = tick;
    _Tick<enable_float> * aim = std::lower_bound(beginPtr, endPtr, o->temp, tickCmp<enable_float>);

    require(aim != endPtr && aim->id == tick, "呜呜");

    std::memmove(aim, aim + 1, sizeof(o->temp) * (endPtr - aim - 1));
    o->length--;
}

template<bool enable_float>
std::pair<_Tick<enable_float> *, bool> nextInitializedTickWithinOneWord(
    Ticks<enable_float> * o,
    int24 tick,
    int24 tickspace,
    bool lte,
    bool newOperation
) {
    static _Tick<enable_float> * cache = NULL;

    _Tick<enable_float> * beginPtr = (&o->temp) + 1;
    _Tick<enable_float> * endPtr   = beginPtr + o->length;
    if(tick < 0 && tick % tickspace != 0) {
        tick = tick / tickspace - 1;
    } else {
        tick /= tickspace;
    }
    bool useCache = (!newOperation && cache != NULL);
    if(useCache) {
        assert(beginPtr <= cache && cache < endPtr);

        if(lte) {
            auto [wordPos, bitPos] = position(tick);

            int tmp = cache->id / tickspace;
            while(tmp > tick) {
                cache--;
                if(cache < beginPtr || GWordPos((tmp = (cache->id / tickspace))) != wordPos){
                    cache = NULL;
                    break;
                }
            }

            if(cache == NULL) {
                o->temp.id = fetchLowerBound(wordPos) * tickspace;
                return std::make_pair(&(o->temp), false);
            } else {
                return std::make_pair(cache, true);
            }
        } else {
            auto [wordPos, bitPos] = position(tick + 1);

            int tmp = cache->id / tickspace;
            while(tmp <= tick) {
                cache++;
                if(cache >= endPtr || GWordPos(tmp = (cache->id / tickspace)) != wordPos) {
                    cache = NULL;
                    break;
                }
            }

            if(cache == NULL) {
                o->temp.id = fetchUpperBound(wordPos) * tickspace;
                return std::make_pair(&(o->temp), false);
            } else {
                return std::make_pair(cache, true);
            }

        }
    } else {
        if(lte) {
            auto [wordPos, bitPos] = position(tick);

            o->temp.id = tick * tickspace;
            _Tick<enable_float> * next = std::upper_bound(beginPtr, endPtr, o->temp, tickCmp<enable_float>);

            if(newOperation) prefetch_range((char*)(&next->id) - 64, 64);

            if(next == beginPtr || GWordPos((next - 1)->id / tickspace) != wordPos){
                o->temp.id = fetchLowerBound(wordPos) * tickspace;
                cache = NULL;
                return std::make_pair(&(o->temp), false);
            } else {
                return std::make_pair(cache = (next - 1), true);
            }
        } else {
            tick += 1;
            auto [wordPos, bitPos] = position(tick);

            o->temp.id = tick * tickspace;
            _Tick<enable_float> * next = std::lower_bound(beginPtr, endPtr, o->temp, tickCmp<enable_float>);

            if(newOperation) prefetch_range((void*)(&next->id), 64);

            if(next == endPtr || GWordPos(next->id / tickspace) != wordPos){
                o->temp.id = fetchUpperBound(wordPos) * tickspace;
                cache =  NULL;
                return std::make_pair(&(o->temp), false);
            } else {
                return std::make_pair(cache = next, true);
            }

        }
    }

}


template<bool enable_float>
std::istream& operator>>(std::istream& is, Ticks<enable_float>& ticks) {
    is >> ticks.length;

    _Tick<enable_float> * beginPtr = (&ticks.temp) + 1;
    _Tick<enable_float> * endPtr   = beginPtr + ticks.length;

    for(auto now = beginPtr; now < endPtr; now++) {
        is >> (*now);
    }
    return is;
}

template<bool enable_float>
std::ostream& operator<<(std::ostream& os, const Ticks<enable_float> &ticks) {
    os << ticks.length << std::endl;
    _Tick<enable_float> * beginPtr = ((_Tick<enable_float> * )(&ticks.temp)) + 1;
    _Tick<enable_float> * endPtr   = beginPtr + ticks.length;

    for(auto i = beginPtr; i < endPtr; i++) {
        os << (*i) << std::endl;
    }
    return os;
}

/*

// have the same API and behaviour with TickBitmap but based on a STL, `std::set`.
class TickBitMapBaseOnVector {
public:
    // To save initialized `tick` (notice: tick is real_tick_No. / tickSpacing)
    std::vector<int24> data;
    std::vector<int24>::iterator cache;
    bool validCache;
#ifdef DEBUG
    int cacheMiss;
    int cacheTotal;
#endif
    TickBitMapBaseOnVector() {
        data.clear();
        validCache = false;
    }
    std::pair<bool, std::vector<int24>::iterator> isExist(int24 tick) {
        auto target = lower_bound(data.begin(), data.end(), tick);
        return make_pair(target != data.end() && *target == tick, target);
    }
    std::pair<int16, int16> position(int24 tick) {
        return std::make_pair(int16(tick >> 8), uint8(tick - (int16(tick >> 8) * 256)));
    }
    void flipTick(
        int24 tick,
        int24 tickSpacing
    ) {
        require(tick % tickSpacing == 0, "QAZ");
        validCache = false;
        // transform the real tick into the image of the tick in the tick space.
        tick /= tickSpacing;
        auto [exist, target] = isExist(tick);
        if(exist) data.erase(target);
        else      data.insert(target, tick);
    }
    bool reachBound(std::vector<int24>::iterator o) { return o == data.end() || o == data.begin(); }
    std::pair<bool, std::vector<int24>::iterator> examCache(int24 tick, bool lte) {
        if(!validCache || reachBound(cache) || reachBound(cache + 1))
            return std::make_pair(false, cache);

        if(lte){ // upper_bound
            return std::make_pair(*(cache - 1) <= tick && *cache >  tick,
                                    cache);
        } else { // lower_bound
            ++cache;
            return std::make_pair(*(cache - 1) < tick && *cache >= tick,
                                    cache);
        }
    }
    std::pair<int24, bool> nextInitializedTickWithinOneWord(
        int24 tick,
        const int24 & tickSpace,
        const bool & lte
    ) {
        // std::cout << "nextInitializedTickWithinOneWord: " << tick << " " << tickSpace << " " << lte << std::endl;
        // transform the real tick into the image of the tick in the tick space.
        if(tick < 0 && tick % tickSpace != 0) {
            // round towards negative infinity
            tick = tick / tickSpace - 1;
        } else {
            tick /= tickSpace;
        }
        std::pair<int24, bool> result;
        if(lte) { // less or equal.
            auto [wordPos, bitPos] = position(tick);
            auto [useCache, next] = examCache(tick, lte);
#ifdef DEBUG
            cacheMiss += !useCache;
            cacheTotal+= 1;
#endif
            if(!useCache)
                cache = (next = upper_bound(data.begin(), data.end(), tick));

            result = (next == data.begin() || GWordPos(*(next - 1)) != wordPos)
                ? std::make_pair(fetchLowerBound(wordPos) * tickSpace, false)
                : std::make_pair(*(next - 1) * tickSpace, true);
        } else { // upper
            tick += 1;
            auto [wordPos, bitPos] = position(tick);
            auto [useCache, next] = examCache(tick, lte);
#ifdef DEBUG
            cacheMiss += !useCache;
            cacheTotal+= 1;
#endif
            if(!useCache)
                cache = (next = lower_bound(data.begin(), data.end(), tick));

            result = (next == data.end() || GWordPos(*next) != wordPos)
                ? std::make_pair(fetchUpperBound(wordPos) * tickSpace, false)
                : std::make_pair(*next * tickSpace, true);
        }
        // std::cerr << cache - data.begin() << " " << lte << std::endl;
        validCache = true;
        return result;
    }
    friend std::istream& operator>>(std::istream& is, TickBitMapBaseOnVector& _) {
        std::vector<int24> &data = _.data;
        data.clear();
        int sz = 0; is >> sz; data.resize(sz);
        for(int i = 0; i < sz; i++) is >> data[i];
        sort(data.begin(), data.end());
        return is;
    }
    friend std::ostream& operator<<(std::ostream& os, const TickBitMapBaseOnVector& _) {
        os << _.data.size() << std::endl;
        for(auto x : _.data) os << x << " ";
        return os << std::endl;
    }
#ifdef DEBUG
    double cacheRate() { return 1 - ((double)cacheMiss / cacheTotal); }
#endif
};
*/

/*
// have the same API and behaviour with TickBitmap but based on a STL, `std::set`.
class TickBitMapBaseOnSet {
    // To save initialized `tick` (notice: tick is real_tick_No. / tickSpacing)
    std::set<int24, std::less<int> > data0; // small to big
    std::set<int24, std::greater<int> > data1; // big to small
public:
    TickBitMapBaseOnSet() { data0.clear(); data1.clear(); }
    bool exist(int24 tick) {
        auto target = data0.lower_bound(tick);
        return target != data0.end() && *target == tick;
    }
    std::pair<int16, int16> position(int24 tick) {
        return std::make_pair(int16(tick >> 8), uint8(tick - (int16(tick >> 8) * 256)));
    }
    void flipTick(
        int24 tick,
        int24 tickSpacing
    ) {
        require(tick % tickSpacing == 0);
        // transform the real tick into the image of the tick in the tick space.
        tick /= tickSpacing;

        if(exist(tick)) data0.erase(tick), data1.erase(tick);
        else            data0.insert(tick), data1.insert(tick);
    }
    std::pair<int24, int24> fetchBound(int24 wordPos) {
        return wordPos != 0 ? std::make_pair(wordPos * 256, wordPos * 256 + 256 - 1) : std::make_pair(0, 255);
    }
    std::pair<int24, bool> nextInitializedTickWithinOneWord(
        int24 tick,
        int24 tickSpace,
        bool lte
    ) {
        // transform the real tick into the image of the tick in the tick space.
        if(tick < 0 && tick % tickSpace != 0) {
            // round towards negative infinity
            tick = tick / tickSpace - 1;
        } else {
            tick /= tickSpace;
        }

        if(lte) { // less or equal.
            auto [wordPos, bitPos] = position(tick);
            auto next = data1.lower_bound(tick);
            return (next == data1.end() || GWordPos(*next) != wordPos)
                ? std::make_pair(fetchLowerBound(wordPos) * tickSpace, false)
                : std::make_pair(*next * tickSpace, true);
        } else { // upper
            tick += 1;
            auto [wordPos, bitPos] = position(tick);
            auto next = data0.lower_bound(tick);
            return (next == data0.end() || GWordPos(*next) != wordPos)
                ? std::make_pair(fetchUpperBound(wordPos) * tickSpace, false)
                : std::make_pair(*next * tickSpace, true);
        }
    }
    friend std::istream& operator>>(std::istream& is, TickBitMapBaseOnSet& _) {
        _.data0.clear(); _.data1.clear();
        int sz = 0; is >> sz;
        while(sz--) {
            int x; is >> x;
            _.data0.insert(x);
            _.data1.insert(x);
        }
        return is;
    }
    friend std::ostream& operator<<(std::ostream& os, const TickBitMapBaseOnSet& _) {
        os << _.data0.size() << std::endl;
        for(auto x : _.data0) os << x << " ";
        return os << std::endl;
    }
};
*/





uint8 mostSignificantBit(uint256 x) {
    // assert(x == "47099173859296676074923436991833339500630638592");
    int r = 0;
    if (x >= uint256("340282366920938463463374607431768211456")) {
        x >>= 128;
        r += 128;
    }
    if (x >= uint256("18446744073709551616")) {
        x >>= 64;
        r += 64;
    }
    if (x >= uint256("4294967296")) {
        x >>= 32;
        r += 32;
    }
    if (x >= uint256("65536")) {
        x >>= 16;
        r += 16;
    }
    if (x >= uint256("256")) {
        x >>= 8;
        r += 8;
    }
    if (x >= uint256("16")) {
        x >>= 4;
        r += 4;
    }
    if (x >= uint256("4")) {
        x >>= 2;
        r += 2;
    }
    if (x >= uint256("2")) r += 1;
    return r;
}

uint8 leastSignificantBit(uint256 x) {
    int r = 255;
    // TODO: [fix] `ttmath` incorrect behaviour when overflow occurs between unsigned variable.
    if ((x & uint128("340282366920938463463374607431768211455"  ))> 0) {
        r -= 128;
    } else {
        x >>= 128;
    }
    if ((x & uint64("18446744073709551615" )) > 0) {
        r -= 64;
    } else {
        x >>= 64;
    }
    if ((x & uint32(4294967295  )) > 0) {
        r -= 32;
    } else {
        x >>= 32;
    }
    if ((x & uint32((1<<16) - 1)) > 0) {
        r -= 16;
    } else {
        x >>= 16;
    }
    if ((x & uint32((1<<8) - 1)) > 0) {
        r -= 8;
    } else {
        x >>= 8;
    }
    if ((x & uint32((1<<4) - 1)) > 0) {
        r -= 4;
    } else {
        x >>= 4;
    }
    if ((x & uint32(3)) > 0) {
        r -= 2;
    } else {
        x >>= 2;
    }
    if ((x & uint32(1)) > 0) r -= 1;
    return r;
}

class TickBitmap {
    std::map<int16, uint256> data;
public:
    std::pair<int16, uint8> position(int24 tick) {
        return std::make_pair(int16(tick >> 8), uint8(tick - (int16(tick >> 8) * 256)));
    }
    /// @notice Flips the initialized state for a given tick from false to true, or vice versa
    /// @param self The mapping in which to flip the tick
    /// @param tick The tick to flip
    /// @param tickSpacing The spacing between usable ticks
    void flipTick(
        int24 tick,
        int24 tickSpacing
    ) {
        // std::cout << tick << " " << tickSpacing << " " << tick / tickSpacing << std::endl;
        require(tick % tickSpacing == 0, "WSX"); // ensure that the tick is spaced
        int16 wordPos; uint8 bitPos;
        std::tie(wordPos, bitPos) = position(tick / tickSpacing);
        // std::cout << wordPos << " " << bitPos << std::endl;
        uint256 mask = uint256(1) << bitPos;
        // data[wordPos].PrintTable(std::cout);
        if (data.find(wordPos) == data.end()) data[wordPos] = 0;
        data[wordPos] ^= mask;
        // data[wordPos].PrintTable(std::cout);
    }
    std::pair<int24, bool> nextInitializedTickWithinOneWord(
        int24 tick,
        int24 tickSpacing,
        bool lte
    ) {
        int24 compressed = tick / tickSpacing;
        if (tick < 0 && tick % tickSpacing != 0) compressed--; // round towards negative infinity

        if (lte) {
            // std::cout << compressed << std::endl;
            auto [wordPos, bitPos] = position(compressed);
            // std::cout << wordPos << " " << bitPos << std::endl;
            // all the 1s at or to the right of the current bitPos
            uint256 mask = (uint256(1) << bitPos) - 1 + (uint256(1) << bitPos);
            if (data.find(wordPos) == data.end()) data[wordPos] = 0;
            uint256 masked = data[wordPos] & mask;
            // mask.PrintTable(std::cout);

            // if there are no initialized ticks to the right of or at the current tick, return rightmost in the word
            bool initialized = masked != 0;
            // overflow/underflow is possible, but prevented externally by limiting both tickSpacing and tick
            int24 next = initialized
                ? (compressed - int24(bitPos - mostSignificantBit(masked))) * tickSpacing
                : (compressed - int24(bitPos)) * tickSpacing;
            return std::make_pair(next, initialized);
            // less and equal.
        } else {
            // start from the word of the next tick, since the current tick state doesn't matter
            auto [wordPos, bitPos] = position(compressed + 1);
            // std::cout << "??? " << compressed + 1 << " " << wordPos << " " << bitPos << std::endl;
            // all the 1s at or to the left of the bitPos
            uint256 mask = ~((uint256(1) << bitPos) - 1);
            if (data.find(wordPos) == data.end()) data[wordPos] = 0;
            uint256 masked = data[wordPos] & mask;

            // if there are no initialized ticks to the left of the current tick, return leftmost in the word
            bool initialized = masked != 0;
            // overflow/underflow is possible, but prevented externally by limiting both tickSpacing and tick
            // std::cout << masked << std::endl;
            int24 next = initialized
                ? (compressed + 1 + int24(int24(leastSignificantBit(masked)) - int24(bitPos))) * tickSpacing
                : (compressed + 1 + int24(((1<<8)-1) - int24(bitPos))) * tickSpacing;
            return std::make_pair(next, initialized);
            // upper.
        }
    }
    void print() {
        for (auto [k, v] : data) {
            for (int i = 0; i < 256; ++i) {
                if ((v & (1<<i)) > 0) {
                    std::cout << ((k<<8) + i) << " ";
                }
            }
        }
        std::cout << std::endl;
    }
    friend std::istream& operator>>(std::istream& is, TickBitmap& tickBitmap) {
        tickBitmap.data.clear();
        int num; is >> num;
        for (int i = 0; i < num; ++i) {
            long long x; is >> x;
            auto [wordPos, bitPos] = tickBitmap.position(x);
            tickBitmap.data[wordPos] |= uint256(1)<<(bitPos);
        }
        return is;
    }
    friend std::ostream& operator<<(std::ostream& os, const TickBitmap& tickBitmap) {
        // std::cerr << tickBitmap.data.size() << std::endl;
        std::vector<long long> res;
        for (auto [k, v] : tickBitmap.data) {
            // std::cerr << k << " " << v << std::endl;
            for (int i = 0; i < 256; ++i) {
                if (((v>>i) & uint256(1)) > 0) {
                    res.push_back(((long long)k<<8)|i);
                }
            }
        }
        os << res.size() << std::endl;
        for (auto x : res) os << x << " ";
        os << std::endl;
        return os;
    }
};

#endif