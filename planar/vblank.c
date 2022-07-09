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

    Che dovrebbe corrispondere a

    .loop	move.l	$dff004,d0
		and.l	#$1ff00,d0
		cmp.l	#303<<8,d0
		bne.b	.loop

    .loop2	move.l	$dff004,d0
            and.l	#$1ff00,d0
            cmp.l	#303<<8,d0
            beq.b	.loop2

    Preso da: http://eab.abime.net/showpost.php?p=657223&postcount=9
*/
static volatile ULONG *custom_vposr = (volatile ULONG *) 0xdff004;
static vb_waitpos;

void wait_vblank() {
    vb_waitpos = 303;   // PAL
    while (((*custom_vposr) & 0x1ff00) != (vb_waitpos<<8)) ;

    // Ci aggiungo anche l'attesa che non sia più in quella riga
    while (((*custom_vposr) & 0x1ff00) == (vb_waitpos<<8)) ;
}