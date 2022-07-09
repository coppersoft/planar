#ifndef _SPRITES_
#define _SPRITES_

#include <clib/exec_protos.h>

// Dati null per sprite inutilizzati, stessa cosa faceva photon
static UWORD __chip NULL_SPRITE_DATA[] = {
    0x0000, 0x0000,
    0x0000, 0x0000
};

void reset_sprites(UWORD* SPR0PTH_addr);
void point_sprite(UWORD* SPRxPTH_addr, UWORD* sprite_data);
void set_sprite_pos_raw(UWORD *sprite_data, UWORD hstart, UWORD vstart, UWORD height);

void set_sprite_pos(UWORD *sprite_data, UWORD hstart, UWORD vstart, UWORD height);

void point_sprite_number(USHORT sprnum, UWORD* sprite_data);

#endif
