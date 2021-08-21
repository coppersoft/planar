#include "bitplanes.h"

/**
    Funzione generica di puntamento bitplane

    @param UBYTE* bitplanes    -> Il puntatore ai bitplane
    @param UWORD* BPL1PTH_addr -> Puntatore al valore in coppertlist di BPL1PTH
    @param int    bpl_number   -> Numero bitplane da puntare
*/
void point_bitplanes (UBYTE* bitplanes, UWORD* BPL1PTH_addr, int bpl_number) {
    ULONG addr;
    int bplptr_offset = 0;
    for (int i = 0; i < bpl_number; i++) {
            addr = (ULONG) &(bitplanes[i * 40]);
            BPL1PTH_addr[bplptr_offset] = (addr >> 16) & 0xffff;
            BPL1PTH_addr[bplptr_offset + 2] = addr & 0xffff;
            bplptr_offset += 4; // next bitplane
    }
}