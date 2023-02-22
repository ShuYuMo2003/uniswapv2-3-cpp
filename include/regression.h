#include <cmath>
#include <cstdio>
#include <algorithm>
#include <vector>

#define BT long double // __float128.
#define ST long double


struct Regression_t{
    BT a, b; // y = a + b * x;
};

std::pair<double, Regression_t> BuildRegression(const std::vector<std::pair<int256, int256>> & _sample) {
    std::vector<std::pair<BT, BT>> sample;
    sample.clear();
    for(const auto & [x, y] : _sample) {
        sample.push_back(std::make_pair(
            (BT)x.ToDouble(),
            (BT)y.ToDouble()
        ));
    }
    BT n = sample.size();
    BT averagex = 0, averagey = 0;
    for(auto & [x, y] : sample) {
        averagex += x;
        averagey += y;
    }
    averagex /= n;
    averagey /= n;

    BT Sqx = 0, Sqy = 0;

    for(auto & [x, y] : sample) {
        Sqx += (x - averagex) * (x - averagex);
        Sqy += (y - averagey) * (y - averagey);
    }

    Sqx /= (n - 1); Sqx = sqrt(Sqx);

    Sqy /= (n - 1); Sqy = sqrt(Sqy);

    BT SM = 0;
    for(auto & [x, y] : sample) {
        SM += ((x - averagex) / Sqx) * ((y - averagey) / Sqy);
    }
    BT r = SM / (n - 1);

    // BT dominator = 0, norminator = 0;
    // for(auto & [x, y] : sample) {
    //     norminator += x * y;
    //     dominator += x * x;
    // }
    // norminator -= n * averagex * averagey;
    // dominator -= n * (averagex * averagex);

    Regression_t result;
    result.b = r * (Sqy / Sqx);

    result.a = averagey - result.b * averagex;

    return std::make_pair(r, result);
}

BT evaluate(const Regression_t & reg, const int256 & _x) {
    BT x = _x.ToDouble();
    return reg.a + reg.b * x;
}
