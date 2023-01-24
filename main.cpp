#include "bigint.hpp"
#include <iostream>

int main() {
    biguint a(3, 100), b(2, 100);
    std::cout << (a >= b) << std::endl;
}