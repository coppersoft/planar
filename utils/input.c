#include "input.h"

void waitfire() {
    volatile UBYTE *ciaa_pra = (volatile UBYTE *) 0xbfe001;
    while ((*ciaa_pra & PRA_FIR1_BIT) != 0) ;
}

BOOL isMousePressed() {
    volatile UBYTE *ciaa_pra = (volatile UBYTE *) 0xbfe001;
    if ((*ciaa_pra & PRA_FIR0_BIT) == 0) {
        return 1;
    } else {
        return 0;
    }
}

void waitmouse()
{
    volatile UBYTE *ciaa_pra = (volatile UBYTE *) 0xbfe001;
    while ((*ciaa_pra & PRA_FIR0_BIT) != 0) ;
}
