#include <cmath>
#include <vector>
#include "types.h"
#include "consts.h"

static long double x[102400], y[102400];

double deviation2v(double a, double b) {
    if(fabs(a - b) < EPS) return 0;
    return fabs((a - b) / std::max(a, b));
}

struct Lagrange{
    std::vector<std::pair<long double, long double> > Pts;
    double Upper;
    bool operator < (const Lagrange & rhs) const { return Upper < rhs.Upper; }
    Lagrange(){}
    void init(const std::vector<std::pair<unsigned int, double> > & sample, int L, int R, int upperid) {
        L = std::max(L, 0);
        R = std::min(R, (int)sample.size() - 1);
        Pts.clear();
        Upper = sample[upperid].first;
        for(int i = L; i <= R; i++) {
            auto now = sample[i];
            Pts.push_back(now);
        }
    }
    double operator() (const double & _x) const {
        long double ret = 0;
        for(int i = 0; i < Pts.size(); i++) {
            long double d = 1;
            for(int j = 0; j < Pts.size(); j++) {
                if(i == j) continue;
                d *= (_x - Pts[j].first) / (Pts[i].first - Pts[j].first);
            }
            ret += d * Pts[i].second;
        }
        return ret;
    }
};