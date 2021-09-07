#include "vblank.h"

#define RAST_LINE_TO_WAIT   (0xc1)      // ok Amiga 1200
//#define RAST_LINE_TO_WAIT   (0x2a)    // ok Amiga 500


/*
    Versione mia presa da Fabio Ciucci
*/
/*void wait_vblank() {

    volatile UBYTE *custom_vposr_low = (volatile UBYTE *) 0xdff005;
    volatile UBYTE *custom_vhposr = (volatile UBYTE *) 0xdff006;

    while ((*custom_vposr_low & 0x01) != 0);

    while ((*custom_vhposr) != RAST_LINE_TO_WAIT);

    while ((*custom_vhposr) == RAST_LINE_TO_WAIT);
}*/

/*
    Versione del corso del cinese.
*/
static volatile ULONG *custom_vposr = (volatile ULONG *) 0xdff004;
static vb_waitpos;

void wait_vblank() {
    vb_waitpos = 303;   // PAL
    while (((*custom_vposr) & 0x1ff00) != (vb_waitpos<<8)) ;
}