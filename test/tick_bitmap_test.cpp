#include "../include/tick_bitmap.h"
#include <iostream>
#include <ctime>

int main(){
    TickBitmap map0;
    TickBitMapBaseOnSet map1;
    srand((unsigned)time(0));
    int space = rand() % 20 + 1;
    for(int t = 0; t <= 1e6; t++) {
        int target = (rand() % 10 + 1) * space;
        if(rand() % 3 == 0) {
            int dir = rand() % 2;
            if(map0.nextInitializedTickWithinOneWord(target, space, dir) \
                == map1.nextInitializedTickWithinOneWord(target, space, dir))
                std::cerr << "passed on test " << t << std::endl;
            else {
                std::cerr << target << " " << space  << " " << dir << std::endl;
            }
        } else {
            map0.flipTick(target, space);
            map1.flipTick(target, space);
        }
    }

    return 0;
}