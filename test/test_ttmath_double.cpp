#include "../lib/ttmath/ttmathint.h"
#include "../lib/ttmath/ttmathuint.h"
#include "../include/types.h"
#include <bitset>


int main(){
    uint160 aim("1145141919810200310030597");
    /////////////1145141919810200309989376.0000000000



    printf("%.10Lf\n", aim.ToDouble());
    printf("%.50Lf\n", aim.X96ToDouble());
}