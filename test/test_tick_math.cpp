#include <iostream>
#include "../include/tickmath.h"

using namespace std;

int main(){
    initializeTicksPrice();
    for(int i = 887271; i <= 887272; i++) {
        cout << i << " " << getSqrtRatioAtTick<uint160>(i) << endl;
    }
    uint160 mid = uint160("1461446703485210103287273052203988822378723970341");
    for(uint160 k = mid - int(1e7); k <= mid + int(1e7); k += int(1e5)) {
        cout << k << " " << getTickAtSqrtRatio(k) << endl;
    }
    return 0;
}

/*
查询的是 1461446703485210103287273052203988822378723970341
*/