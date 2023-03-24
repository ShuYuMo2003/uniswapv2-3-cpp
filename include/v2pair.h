#include "types.h"
#include "../lib/ttmath/ttmathint.h"
#include "../lib/ttmath/ttmathuint.h"


namespace v2{

enum EventType {CRET, SET};

struct V2Event{
    int type;
    std::string address, token0, token1;
    unsigned long long idxHash;
    double reserve0, reserve1;
};


class V2Pair{
public:
    double reserve[2] = {0};
    std::string token[2];
    unsigned long long latestIdxHash;
    V2Pair(const std::string & token0, const std::string & token1, const double & reserve0, const double & reserve1, const unsigned long long & idx) { // 用于创建 Pair.
        token[0] = token0;
        token[1] = token1;
        reserve[0] = reserve0;
        reserve[1] = reserve1;
        latestIdxHash = idx;
    }
    void processEvent(const V2Event & e){
        assert(latestIdxHash <= e.idxHash);
        assert(e.type == SET);
        if(e.type == SET) {
            reserve[0] = e.reserve0;
            reserve[1] = e.reserve1;
            std::cerr << "Set " << e.address << " to " <<  reserve[0] << "  " <<  reserve[1] << std::endl;
        }
        latestIdxHash = e.idxHash;
    }
    double query(bool zeroToOne, double amountIn) {
        if(zeroToOne ? amountIn + 10 > reserve[0] : amountIn + 10 > reserve[1])
            return -1;
        amountIn = amountIn * 997;
        double numerator, denominator;
        if (zeroToOne) {
            numerator = amountIn * reserve[1];
            denominator = reserve[0] * 1000 + amountIn;
        } else {
            numerator = amountIn * reserve[0];
            denominator = reserve[1] * 1000 + amountIn;
        }
        return numerator / denominator;
    }
};

V2Event rawdata2event(std::istringstream & is) {
    static char opt[100];

    static std::string temp;
    static int128 tempn;

    V2Event result;

    assert(is >> opt);
    assert(opt[0] == '2');

    if(opt[1] == 'c') { // CREAT
        result.type = CRET;
        is >> result.address >> result.token0 >> result.token1 >> result.idxHash;
    } else if(opt[1] == 's') { // SET
        result.type = SET;
        is >> result.address;
        is >> tempn; result.reserve0 = tempn.ToDouble();
        is >> tempn; result.reserve1 = tempn.ToDouble();
        is >> result.idxHash;
    }
    return result;
}


}