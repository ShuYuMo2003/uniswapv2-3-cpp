#include <cmath>
#include "type.h"
#include "consts.h"


struct Regression_t{
    FloatType upper;
    long double a, b; // y = a + b * x;
};

void BuildRegression(const int256 * samplex, const int256 * sampley, size_t n, Regression_t * result) {
    static long double x[REGSAMAX], y[REGSAMAX];
    long double avex = 0, avey = 0, sampleSize = n;
    for(int i = 0; i < n; i++) {
        avex += (x[i] = samplex[i].ToDouble());
        avey += (y[i] = sampley[i].ToDouble());
    }
    avex /= sampleSize;
    avey /= sampleSize;

    long double norminator = 0, dominator = 0;
    for(int i = 0; i < n; i++) {
        norminator += x[i] * y[i];
        dominator  += x[i] * x[i];
    }

    norminator -= sampleSize * avex * avey;
    dominator  -= sampleSize * avex * avex;
    result->b = norminator / dominator;
    result->a = avey - result->b * avex;
}

__attribute__((always_inline)) long double evaluate(const Regression_t * reg, const int256 & _x) {
    return reg->a + reg->b * _x.ToDouble();
}
