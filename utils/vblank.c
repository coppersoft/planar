#include "vblank.h"

//#define RAST_LINE_TO_WAIT   (0xc1)
#define RAST_LINE_TO_WAIT   (0x2a)

// 303 è PAL, se NTSC 262
/*void wait_vblank()
{
    while (((*custom_vposr) & 0x1ff00) != (303<<8)) ;
}*/

void wait_vblank() {

    volatile UBYTE *custom_vposr_low = (volatile UBYTE *) 0xdff005;
    volatile UBYTE *custom_vhposr = (volatile UBYTE *) 0xdff006;

    while ((*custom_vposr_low & 0x01) != 0);

    while ((*custom_vhposr) != RAST_LINE_TO_WAIT);

    while ((*custom_vhposr) == RAST_LINE_TO_WAIT);
}