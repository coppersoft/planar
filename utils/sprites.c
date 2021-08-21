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