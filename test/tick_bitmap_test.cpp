#include "../include/ticks.h"
#include <iostream>
#include <cstdlib>
#include <ctime>

int main(){
    TickBitmap map0;

    Ticks<false> *map1 = (Ticks<false>*)malloc((sizeof(_Tick<false>) * 1e6));
    *map1 = Ticks<false>();
    printf("%p\n", map1);

    srand(2003 ^ 1006);
    int space = rand() % 50 + 1;
    for(int t = 0; t <= 1e6; t++) {
        int target = (rand() % 2 ? -1 : 1) * (rand() % 1000) * space;
        if(rand() % 3 == 0) {
            int dir = rand() % 2;
            auto ret0 = map0.nextInitializedTickWithinOneWord(target, space, dir);
            auto ret1 = nextInitializedTickWithinOneWord(map1, target, space, dir);
            if(ret0.second == ret1.second && ret0.first == ret1.first->id) {
                if(t % 1000 == 0) std::cerr << "passed on test " << t << std::endl;
            } else {
                std::cerr << ret0.first << " " << ret0.second << std::endl;
                std::cerr << ret1.first->id << " " << ret1.second << std::endl;
                std::cerr << target << " " << space  << " " << dir << std::endl;
                assert(false);
            }
        } else {
            map0.flipTick(target, space);
            flipTick(map1, target, space);
        }
    }

    free(map1);
    return 0;
}