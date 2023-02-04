#include "../include/tick_bitmap.h"
#include <iostream>
#include <ctime>

int main(){
    TickBitmap map0;
    TickBitMapBaseOnSet map1;
    TickBitMapBaseOnVector map2;
    srand(233);
    int space = rand() % 50 + 1;
    for(int t = 0; t <= 1e7; t++) {
        int target = (rand() % 2 ? -1 : 1) * (rand() % 1000) * space;
        if(rand() % 3 == 0) {
            int dir = rand() % 2;
            if(map0.nextInitializedTickWithinOneWord(target, space, dir) \
                == map1.nextInitializedTickWithinOneWord(target, space, dir) &&
               map1.nextInitializedTickWithinOneWord(target, space, dir) \
                == map2.nextInitializedTickWithinOneWord(target, space, dir)) {
                if(t % 10000 == 0) std::cerr << "passed on test " << t << std::endl;
            } else {
                std::cerr << target << " " << space  << " " << dir << std::endl;
                assert(false);
            }
        } else {
            map0.flipTick(target, space);
            map1.flipTick(target, space);
            map2.flipTick(target, space);
        }
    }

    return 0;
}