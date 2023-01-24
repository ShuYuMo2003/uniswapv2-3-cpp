#ifndef headerfiletickbitmap
#define headerfiletickbitmap

#include <map>

#include "types.h"
#include "bitmath.h"

class TickBitmap {
    std::map<int16, uint256> data;
public:
    std::pair<int16, uint8> position(int24 tick) {
        return std::make_pair(int16(tick >> 8), uint8(tick % 256));
    }
    /// @notice Flips the initialized state for a given tick from false to true, or vice versa
    /// @param self The mapping in which to flip the tick
    /// @param tick The tick to flip
    /// @param tickSpacing The spacing between usable ticks
    void flipTick(
        int24 tick,
        int24 tickSpacing
    ) {
        require(tick % tickSpacing == 0); // ensure that the tick is spaced
        int16 wordPos; uint8 bitPos;
        std::tie(wordPos, bitPos) = position(tick / tickSpacing);
        uint256 mask = uint256(1) << bitPos;
        data[wordPos] ^= mask;
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
            uint256 masked = data[wordPos] & mask;

            // if there are no initialized ticks to the right of or at the current tick, return rightmost in the word
            bool initialized = masked != 0;
            // overflow/underflow is possible, but prevented externally by limiting both tickSpacing and tick
            int24 next = initialized
                ? (compressed - int24(bitPos - mostSignificantBit(masked))) * tickSpacing
                : (compressed - int24(bitPos)) * tickSpacing;
            return std::make_pair(next, initialized);
        } else {
            // start from the word of the next tick, since the current tick state doesn't matter
            auto [wordPos, bitPos] = position(compressed + 1);
            // all the 1s at or to the left of the bitPos
            uint256 mask = ~((uint256(1) << bitPos) - 1);
            uint256 masked = data[wordPos] & mask;

            // if there are no initialized ticks to the left of the current tick, return leftmost in the word
            bool initialized = masked != 0;
            // overflow/underflow is possible, but prevented externally by limiting both tickSpacing and tick
            int24 next = initialized
                ? (compressed + 1 + int24(leastSignificantBit(masked) - bitPos)) * tickSpacing
                : (compressed + 1 + int24(((1<<8)-1) - bitPos)) * tickSpacing;
            return std::make_pair(next, initialized);
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
};

#endif