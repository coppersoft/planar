#ifndef _SCREEN_
#define _SCREEN_

#include <clib/exec_protos.h>
#include "ahpc_registers.h"

// Diwstart e stop
#define DIWSTRT_VALUE      0x2c81
#define DIWSTOP_VALUE_PAL  0x2cc1

// Data fetch
#define DDFSTRT_VALUE      0x0038
#define DDFSTOP_VALUE      0x00d0

// copper instruction macros
#define COP_MOVE(addr, data) addr, data
#define COP_WAIT_END  0xffff, 0xfffe

// Indici array copperlist
#define BPLCON0_VALUE_IDX (43)
#define BPL1PTH_VALUE_IDX (49)
#define SPR0PTH_VALUE_IDX (3)
#define BPL1MOD_VALUE_IDX (45)
#define BPL2MOD_VALUE_IDX (47)

// 5 bitplanes, composite color.
#define BPLCON0_5BPP_COMPOSITE_COLOR 0x5200

struct AmigaScreen {
    UWORD   bytes_width;
    UWORD   height;
    UBYTE   depth;
    UBYTE   left_margin_bytes;
    UBYTE   right_margin_bytes;
    UBYTE   display_start_offset_bytes;
    BOOL    dual_playfield;
    UBYTE*  bitplanes;
    ULONG   framebuffer_size;
    UWORD   row_size;
};

extern struct AmigaScreen screen;

/*
    5:00
    __chip dice a vbcc di mettere la roba in chip, ovviamente per la coppelist in questo caso

    "la prima move dice al computer di emettere nello schermo i colori registrati nel color register zero
    lo facciamo settando il color burst bit in BPLCON0"

    http://amiga-dev.wikidot.com/hardware:bplcon0  (bit 9, infatti vedi BPLCON0_COMPOSITE_COLOR)

*/

extern __chip UWORD copperlist[];


void init_screen(UWORD   bytes_width,
    UWORD   height,
    UBYTE   depth,
    UBYTE   left_margin_bytes,
    UBYTE   right_margin_bytes,
    UBYTE   display_start_offset_bytes,
    BOOL    dual_playfield);

void fadePaletteOCS(UWORD* rawPalette,UWORD* copPalette,USHORT frame,USHORT colors);


#endif
