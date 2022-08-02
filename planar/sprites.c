#include "sprites.h"
#include "screen.h"

/**
 * Resetta tutti gli sprites alla struttura null
 * 
 * @param UWORD* SPR0PTH_addr -> Puntatore al valore in coppertlist di SPR0PTH
 */
void reset_sprites(UWORD* SPR0PTH_addr) {
    // Punto gli sprite 0-7 a null
    for (int i = 0; i < 8; i++) {
        SPR0PTH_addr[i*4] = (((ULONG) NULL_SPRITE_DATA) >> 16) & 0xffff;
        SPR0PTH_addr[i*4+2] = ((ULONG) NULL_SPRITE_DATA) & 0xffff;
    }
}

/**
 * Punto i dati dello sprite X in copperlist
 * 
 * @param UWORD* SPRxPTH_addr -> Puntatore al valore in copperlist dello sprite x (0-7)
 * @param UWORD* sprite_data  -> Puntatore alla struttura dati dello sprite
 */
void point_sprite(UWORD* SPRxPTH_addr, UWORD* sprite_data) {
    SPRxPTH_addr[0] = ((ULONG)sprite_data >> 16) & 0xffff;
    SPRxPTH_addr[2] = (ULONG)sprite_data & 0xffff;
}

/**
 * Punto i dati dello sprite X in copperlist
 * 
 * USHORT sprnum -> Numero dello sprite [0..7]
 * @param UWORD* sprite_data  -> Puntatore alla struttura dati dello sprite
 */
void point_sprite_number(USHORT sprnum, UWORD* sprite_data) {
    UWORD* sprXpth = &copperlist[SPR0PTH_VALUE_IDX];

    sprXpth += sprnum*4;

    point_sprite(sprXpth,sprite_data);

    //sprXpth[0] = ((ULONG)sprite_data >> 16) & 0xffff;
    //sprXpth[2] = (ULONG)sprite_data & 0xffff;
}


void set_sprite_pos_raw(UWORD *sprite_data, UWORD hstart, UWORD vstart, UWORD height)
{
    UWORD vstop = vstart + height;
    sprite_data[0] = ((vstart & 0xff) << 8) | ((hstart >> 1) & 0xff);
    // vstop + high bit of vstart + low bit of hstart
    sprite_data[1] = ((vstop & 0xff) << 8) |  // vstop 8 low bits
        ((vstart >> 8) & 1) << 2 |  // vstart high bit
        ((vstop >> 8) & 1) << 1 |   // vstop high bit
        (hstart & 1) |              // hstart low bit
        sprite_data[1] & 0x80;      // preserve attach bit
}

void set_sprite_pos(UWORD *sprite_data, UWORD hstart, UWORD vstart, UWORD height)
{
    hstart += 128;
    vstart += 44;   // 0x2c, vedi DIWSTRT
    set_sprite_pos_raw(sprite_data,hstart,vstart,height);
}