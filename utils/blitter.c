#include "blitter.h"
#include <clib/graphics_protos.h>
#include <hardware/custom.h>
#include "disk.h"

extern struct Custom custom;

/**
 *  Semplice funzione di blitting A -> D senza cookie cut
 *  ATTENZIONE: Al momento assume che la larghezza dei bitplane sia di 40 byte (320px)
 * 
 *  @param  UBYTE* source       -> Indirizzo sorgente
 *  @param  UBYTE* dest         -> Indirizzo destinazione
 *  @param  int    words        -> Larghezza in words della sorgente 
 *  @param  int    rows         -> Numero di righe della sorgente
 *  @param  int    bitplanes    -> Numero bitplanes
 */
void simple_blit(UBYTE* source, UBYTE* dest, int words, int rows, int bitplanes) {
    OwnBlitter();
    WaitBlit();
    /* 0 = shift nullo
       9 = 1001: abilito solo i canali A e D
       f0 = minterm, copia semplice
    */
    custom.bltcon0 = 0x09f0;    // dff040
    custom.bltcon1 = 0x0;       // dff042

    custom.bltapt = source;
    custom.bltdpt = dest;

    custom.bltafwm = 0xffff;    // Maschere
    custom.bltalwm = 0xffff;

    custom.bltamod = 0;         // Modulo 0 in A
    custom.bltdmod = 40 - (words*2);

    custom.bltsize = (UWORD) ((rows*bitplanes) << 6) | words;
    DisownBlitter();
}

void masked_blit(UBYTE* source, UBYTE* dest, UBYTE* mask, UBYTE* background, int x, int y, int words, int rows, int bitplanes) {
    OwnBlitter();
    WaitBlit();

    int shift = x & 0xf;
    int horiz_bytes_offset = (x >> 4)*2;

    UWORD   bltcon0_value = 0x0fe2;     // 0 shift momentaneamente, f tutti i canali
                                        // e2 i minterm per cookie cut
    UWORD   bltcon1_value = 0x0;

    shift = shift << 12;

    bltcon0_value |= shift;
	bltcon1_value |= shift;

    custom.bltcon0 = bltcon0_value;
    custom.bltcon1 = bltcon1_value;

    custom.bltafwm = 0xffff;    // Maschere
    custom.bltalwm = 0xffff;

    custom.bltapt = source;
    custom.bltbpt = mask;

    int bytes_offset = ((40*y)*bitplanes)+horiz_bytes_offset;

    // ?????

    dest += bytes_offset;
    background += bytes_offset;

    custom.bltcpt = background;
    custom.bltdpt = dest;

    custom.bltamod = 0;
    custom.bltbmod = 0;

    int moduloc = 40 - (words*2);       // Al momento cablo 40
    int modulod = moduloc;              // Al momento è uguale

    custom.bltcmod = moduloc;
    custom.bltdmod = modulod;

    custom.bltsize = (UWORD) ((rows*bitplanes) << 6) | words;
    DisownBlitter();
}

BlitterBob init_bob(char* img_file, char* mask_file, int words, int rows, int bitplanes) {
    BlitterBob newbob;

    newbob.header.bitplanes = bitplanes;
    newbob.header.firstdraw = 1;
    newbob.header.rows = rows;
    newbob.header.words = words;

    size_t size = (words*2)*rows*bitplanes;

    newbob.imgdata = alloc_and_load_asset(size,img_file);
    newbob.mask = alloc_and_load_asset(size,mask_file);

    return newbob;
}

void draw_bob(BlitterBob* bob,UBYTE* dest, int x,int y) {
    if(!(bob->header.firstdraw)) {
        // TODO: Ripristinare lo sfondo con una bella blittata partendo dagli x e y attuali prima dello
        //       spostamento
    }
    bob->header.x = x;
    bob->header.y = y;
    bob->header.firstdraw = 0;
    
    masked_blit(bob->imgdata,dest,bob->mask,dest,x,y,bob->header.words,bob->header.rows,bob->header.bitplanes);
}
