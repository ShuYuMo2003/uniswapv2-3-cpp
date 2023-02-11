#include <iostream>
#include "../include/tickmath.h"

using namespace std;

int main(){
    initializeTicksPrice();
    // cout << getTickAtSqrtRatio(16850.2) << endl;
    // cout << getTickAtSqrtRatio(16850.1) << endl;
    // cout << getTickAtSqrtRatio(16850.0) << endl;
    // cout << getTickAtSqrtRatio(16849.9) << endl;
    // cout << getTickAtSqrtRatio(16849.7) << endl;
    // cout << getTickAtSqrtRatio(16849.6) << endl;
    // cout << getTickAtSqrtRatio(16849.5) << endl;
    // cout << getTickAtSqrtRatio(16849.4) << endl;
    // cout << getTickAtSqrtRatio(16849.2) << endl;
    // cout << getTickAtSqrtRatio(16849.1) << endl;
    // cout << getTickAtSqrtRatio(16849.0) << endl;
    // cout << getTickAtSqrtRatio(16848.9) << endl;
    // cout << getTickAtSqrtRatio(16848.8) << endl;
    uint160 Price = uint160("1592294147891175336578686644750566");
    std::cout << std::setiosflags(std::ios::fixed) << std::setprecision(7);
    cout << Price.X96ToDouble() << std::endl;
    cout << getTickAtSqrtRatio(Price) << endl; // 198176

    cout << "======================= FloatTick ====================" << endl;
    for(double now = 20097.3783921; now <= 20097.7783921; now += 0.01)
        cout << now << " " << getTickAtSqrtRatio(now) << endl; // 198177
    return 0;
}