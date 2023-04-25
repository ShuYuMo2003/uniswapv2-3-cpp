#include "../include/logger.h"

int main(){
    int a = 1000;
    Logger(std::cout, INFO, "writer") << "MSG = " << a << kkl();
    return 0;
}