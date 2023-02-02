#ifndef headerfiletransferhelper
#define headerfiletransferhelper

#include <iostream>

#include "types.h"

/// @notice Transfers tokens from msg.sender to a recipient
/// @dev Calls transfer on token contract, errors with TF if transfer fails
/// @param token The contract address of the token which will be transferred
/// @param to The recipient of the transfer
/// @param value The value of the transfer
void safeTransfer(
    address token,
    address to,
    uint256 value
) {
    // std::cout << "Transfer " << value << " ";
    // token.PrintTable(std::cout);
    // std::cout << " to ";
    // to.PrintTable(std::cout);
    // std::cout << std::endl;
    // (bool success, bytes memory data) = token.call(
    //     abi.encodeWithSelector(IERC20Minimal.transfer.selector, to, value)
    // );
    // require(success && (data.length == 0 || abi.decode(data, (bool))), 'TF');
}

#endif