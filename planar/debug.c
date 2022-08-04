#include "debug.h"
#include "vblank.h"
#include <stdio.h>

void wait(int sec) {
    sec *= 50;
    for (int i=0 ; i < sec; i++) {
        wait_vblank();
    }
}

void printCopperlist(UWORD* copperlist) {
	int index = 0;
	
	while (1) {
        UWORD w1,w2;
        w1 = *copperlist++;
        w2 = *copperlist++;
        printf("%d: %#06x, %#06x\n",index, w1,w2);
        index+=2;
        if (w1 == 0xffff) break;
    }
}