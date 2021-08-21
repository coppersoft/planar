#include "sprites.h"

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