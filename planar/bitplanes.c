#include "bitplanes.h"
#include "screen.h"
#include <clib/exec_protos.h>


extern int drawBufferSelector;

static void point_bitplanes_dp (UBYTE* bitplanes, UWORD* BPL1PTH_addr, int bpl_number) {
    ULONG addr;
    int bplptr_offset = 0;

    // TODO: test con 8 bitplane, generalizzare
    // Playfield 1, BPL 1,3,5,7 con prima metà del buffer
    for (int i=0 ; i < 4;i++) {
        addr = (ULONG) &(bitplanes[i * screen.row_size]);
        BPL1PTH_addr[bplptr_offset] = (addr >> 16) & 0xffff;
        BPL1PTH_addr[bplptr_offset + 2] = addr & 0xffff;
        bplptr_offset += 8; // next bitplane, salto uno per il DP
    }
    
    UWORD* BPL2PTH_addr = BPL1PTH_addr+4; // Trovo BPL2PTH (ATTENZIONE! E' +4 WORD!!!)
    
    bitplanes += screen.framebuffer_size / 2;       // Prossimo playfield
    /* printf("point_bitplanes_dp, screen.framebuffer_size /s %d\n",screen.framebuffer_size / 2);
    printf("point_bitplanes_dp, secondo pf bitplanes %d\n",bitplanes); */
    bplptr_offset = 0;
    // Playfield 2, BPL 2,4,6,8 con seconda metà del buffer
    for (int i=0; i < 4; i++) {
        addr = (ULONG) &(bitplanes[i * screen.row_size]);
        BPL2PTH_addr[bplptr_offset] = (addr >> 16) & 0xffff;
        BPL2PTH_addr[bplptr_offset + 2] = addr & 0xffff;
        bplptr_offset += 8; // next bitplane, salto uno per il DP
    }
}

/**
    Funzione generica di puntamento bitplane

    @param UBYTE* bitplanes    -> Il puntatore ai bitplane
    @param UWORD* BPL1PTH_addr -> Puntatore al valore in coppertlist di BPL1PTH
    @param int    bpl_number   -> Numero bitplane da puntare
*/
void point_bitplanes (UBYTE* bitplanes, UWORD* BPL1PTH_addr, int bpl_number) {

    if (screen.dual_playfield) {
        point_bitplanes_dp(bitplanes,BPL1PTH_addr,bpl_number);
        return;
    }

    ULONG addr;
    int bplptr_offset = 0;
    for (int i = 0; i < bpl_number; i++) {
            addr = (ULONG) &(bitplanes[i * screen.row_size]);
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

void switchBuffers() {
    int bitplaneOffset = 0;
    // Se il vecchio drawbuffer è il primo dei buffer, allora l'offset rimane zero
    if (drawBufferSelector == 0) {
        drawBufferSelector = 1;
    } else {
        bitplaneOffset = screen.framebuffer_size;
        drawBufferSelector = 0;
    }
    point_bitplanes(screen.bitplanes+bitplaneOffset+screen.display_start_offset_bytes,&copperlist[BPL1PTH_VALUE_IDX],screen.depth);
    
}