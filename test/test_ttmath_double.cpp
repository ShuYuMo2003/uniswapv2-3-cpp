#include "../lib/ttmath/ttmathint.h"
#include "../lib/ttmath/ttmathuint.h"
#include "../include/types.h"
#include <bitset>


int main(){
    uint160 aim("123456712398462189123456789");


    printf("%.10lf\n", aim.ToDouble());
    // printf("%.50lf\n", aim.X96ToDouble());
}