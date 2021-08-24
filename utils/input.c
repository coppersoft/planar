#include "input.h"

void waitfire() {
    volatile UBYTE *ciaa_pra = (volatile UBYTE *) 0xbfe001;
    while ((*ciaa_pra & PRA_FIR1_BIT) != 0) ;
}
