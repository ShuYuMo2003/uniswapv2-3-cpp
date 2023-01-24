#ifndef headerfileposition
#define headerfileposition

#include <map>

#include "consts.h"
#include "fullmath.h"
#include "liquiditymath.h"
#include "types.h"
#include "util.h"

struct Position {
    // the amount of liquidity owned by this position
    uint128 liquidity;
    // fee growth per unit of liquidity as of the last update to liquidity or fees owed
    uint256 feeGrowthInside0LastX128;
    uint256 feeGrowthInside1LastX128;
    // the fees owed to the position owner in token0/token1
    uint128 tokensOwed0;
    uint128 tokensOwed1;

    /// @notice Credits accumulated fees to a user's position
    /// @param self The individual position to update
    /// @param liquidityDelta The change in pool liquidity as a result of the position update
    /// @param feeGrowthInside0X128 The all-time fee growth in token0, per unit of liquidity, inside the position's tick boundaries
    /// @param feeGrowthInside1X128 The all-time fee growth in token1, per unit of liquidity, inside the position's tick boundaries
    void update(
        int128 liquidityDelta,
        uint256 feeGrowthInside0X128,
        uint256 feeGrowthInside1X128
    ) {
        uint128 liquidityNext;
        if (liquidityDelta == 0) {
            require(liquidity > 0, "NP"); // disallow pokes for 0 liquidity positions
            liquidityNext = liquidity;
        } else {
            liquidityNext = addDelta(liquidity, liquidityDelta);
        }

        // calculate accumulated fees
        uint128 _tokensOwed0 = uint128(
            mulDiv(feeGrowthInside0X128 - feeGrowthInside0LastX128, liquidity, Q128)
        );
        uint128 _tokensOwed1 = uint128(
            mulDiv(feeGrowthInside1X128 - feeGrowthInside1LastX128, liquidity, Q128)
        );

        // update the position
        if (liquidityDelta != 0) liquidity = liquidityNext;
        feeGrowthInside0LastX128 = feeGrowthInside0X128;
        feeGrowthInside1LastX128 = feeGrowthInside1X128;
        if (_tokensOwed0 > 0 || _tokensOwed1 > 0) {
            // overflow is acceptable, have to withdraw before you hit type(uint128).max fees
            tokensOwed0 += _tokensOwed0;
            tokensOwed1 += _tokensOwed1;
        }
    }

    void print() {
        std::cout << liquidity << " "
            << feeGrowthInside0LastX128 << " " << feeGrowthInside1LastX128 << " "
            << tokensOwed0 << " " << tokensOwed1 << std::endl;
    }
};

struct Positions {
    std::map<std::tuple<address, int24, int24>, Position> data;
    /// @notice Returns the Info struct of a position, given an owner and position boundaries
    /// @param self The mapping containing all user positions
    /// @param owner The address of the position owner
    /// @param tickLower The lower tick boundary of the position
    /// @param tickUpper The upper tick boundary of the position
    /// @return position The position info struct of the given owners' position
    Position& get(
        address owner,
        int24 tickLower,
        int24 tickUpper
    ) {
        return data[std::make_tuple(owner, tickLower, tickUpper)];
    }
};


#endif