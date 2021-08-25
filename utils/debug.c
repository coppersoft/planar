#include "debug.h"
#include "vblank.h"

void wait(int sec) {
    sec *= 50;
    for (int i=0 ; i < sec; i++) {
        wait_vblank();
    }
}