#include <cmath>
#include <vector>
#include "types.h"
#include "consts.h"

static long double x[102400], y[102400];

double deviation2v(double a, double b) {
    if(fabs(a - b) < EPS) return 0;
    return fabs((a - b) / std::max(a, b));
}

struct Regression_t{
    FloatType upper;
    long double a, b; // y = a + b * x;
    bool operator < (const Regression_t & rhs) const {
        return upper < rhs.upper;
    }
    Regression_t(){}
    Regression_t(std::vector<std::pair<unsigned int, double> > & sample, int L, int R) {
        upper = sample[R].first;
        int n = R - L + 1;
        long double avex = 0, avey = 0, sampleSize = n;
        for(int i = L; i <= R; i++) {
            avex += (x[i] = sample[i].first);
            avey += (y[i] = sample[i].second);
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
        b = norminator / dominator;
        a = avey - b * avex;
    }
    double operator() (const double & _x) const {
        return a + b * _x;
    }
    double deviation(std::vector<std::pair<unsigned int, double> > & sample, int L, int R) {
        double dmax = -1;
        for(int i = L; i <= R; i++) {
            dmax = std::max(dmax, deviation2v(sample[i].second, a + b * sample[i].first));
        }
        return dmax;
    }
};