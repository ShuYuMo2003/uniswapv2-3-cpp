#ifndef headerfiletickbitmap
#define headerfiletickbitmap

#include <map>

#include "types.h"
#include "bitmath.h"
#include "util.h"
#include <set>

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
    #define GWordPos(a) ((a) >> 8)
    #define GBitPos(a) abs(a % 256)
    std::pair<int16, int16> position(int24 tick) {
        return std::make_pair(tick >> 8, abs(tick % 256));
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
                ? std::make_pair((tick - bitPos) * tickSpace, false)
                : std::make_pair(*next * tickSpace, true);
        } else { // upper
            tick += 1;
            auto [wordPos, bitPos] = position(tick);
            auto next = data0.lower_bound(tick);
            return (next == data0.end() || GWordPos(*next) != wordPos)
                ? std::make_pair((tick + 255 - bitPos) * tickSpace, false)
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
        require(tick % tickSpacing == 0); // ensure that the tick is spaced
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
            // all the 1s at or to the left of the bitPos
            uint256 mask = ~((uint256(1) << bitPos) - 1);
            if (data.find(wordPos) == data.end()) data[wordPos] = 0;
            uint256 masked = data[wordPos] & mask;

            // if there are no initialized ticks to the left of the current tick, return leftmost in the word
            bool initialized = masked != 0;
            // overflow/underflow is possible, but prevented externally by limiting both tickSpacing and tick
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
            // std::cerr << "??? " << x << std::endl;
            tickBitmap.data[x>>8] |= uint256(1)<<(x%256);
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