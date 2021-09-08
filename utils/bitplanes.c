#include "bitplanes.h"
#include <clib/exec_protos.h>


extern int drawBufferSelector;

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

UBYTE* init_bitplanes(size_t size) {
    size*=2;                                    
    UBYTE* bpls = AllocVec(size,MEMF_CHIP|MEMF_CLEAR);
    return bpls;
}

void free_bitplanes(UBYTE* bitplanes) {
    FreeVec(bitplanes);
}

void switchBuffers(UBYTE* bitplanes, UWORD* BPL1PTH_addr, int bpl_number, size_t bpls_size) {
    int bitplaneOffset = 0;
    // Se il vecchio drawbuffer è il primo dei buffer, allora l'offset rimane zero
    if (drawBufferSelector == 0) {
        drawBufferSelector = 1;
    } else {
        bitplaneOffset = bpls_size;
        drawBufferSelector = 0;
    }
    
    point_bitplanes(bitplanes+bitplaneOffset,BPL1PTH_addr,bpl_number);
}