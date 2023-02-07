#include <iostream>
#include <cstdio>
#include "../include/types.h"
#include "../include/pool.h"

using namespace std;

std::pair<double, double> swap_handle(const Pool & pool, bool zeroToOne, double amountIn) {
    static double SQPRL = uint160("4295128740").X96ToDouble();
    static double SQPRR = uint160("1461446703485210103287273052203988822378723970341").X96ToDouble();
    return pool.swap_effectless(zeroToOne, amountIn, zeroToOne ? SQPRL : SQPRL);
}

std::pair<int256, int256> raw_swap_handle(const Pool & pool, bool zeroToOne, int256 amountIn) {
    static double SQPRL = uint160("4295128740").X96ToDouble();
    static double SQPRR = uint160("1461446703485210103287273052203988822378723970341").X96ToDouble();
    return pool.swap("0x0", zeroToOne, amountIn, zeroToOne ? SQPRL : SQPRL, "", false);
}

const int UNI_DATA_SIZE = 150;

struct testcase{
    bool zeroToOne;
    int256 raw_amount;
    double amount;
    std::pair<double, double> result;
} tc[UNI_DATA_SIZE];

void generate(const Pool & pool) {
    srand(20031006);
    for(int i = 0; i < UNI_DATA_SIZE; i++) {
        tc[i].zeroToOne  =  rand() % 2;
        tc[i].raw_amount =  (uint160(1) << i);
        tc[i].amount     =  tc[i].raw_amount.ToDouble();
        std::pair<double, double> ret = raw_swap_handle(pool, tc[i].zeroToOne, tc[i].raw_amount);
        tc[i].result     = [ret.first.ToDouble(), ret.second.ToDouble()];
    }
}

int main(){
    initializeTicksPrice();
    return 0;
}
