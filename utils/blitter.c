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
    //printf("blitto, offset %d\n",bytes_offset);
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
    newbob.prev_background = AllocMem(size,MEMF_CHIP|MEMF_CLEAR);

    return newbob;
}

/*
    TODO: Generalizzare il 40!!!
*/
static int getOffset(BlitterBob* bob) {
    ////printf("getoffset:\n");
    int y = bob->header.y;
    int x = bob->header.x;
    int bitplanes = bob->header.bitplanes;

    ////printf("x: %d\n",x);
    ////printf("y: %d\n",y);
    ////printf("bitplanes: %d\n",bitplanes);

    int offset = y*40*bitplanes;
    offset += (x>> 4)*2;
    ////printf("Valore calcolato dentro getoffset %d\n",offset);
    return offset;
}

static void save_background(BlitterBob* bob,UBYTE* source) {
    
    int offset = getOffset(bob);

    source+=offset;

    //printf("save_background, offset %d\n",getOffset(bob));

    int words = bob->header.words;
    int rows = bob->header.rows;
    int bitplanes = bob->header.bitplanes;

/*
    //printf("save_background \n");
    //printf("words: %d\n",words);
    //printf("rows: %d\n",rows);
    //printf("bitplanes: %d\n",bitplanes);
    //printf("offset: %d\n",getOffset(bob));
    */

    OwnBlitter();
    WaitBlit();
    /* 0 = shift nullo
       9 = 1001: abilito solo i canali A e D
       f0 = minterm, copia semplice
    */
    custom.bltcon0 = 0x09f0;    // dff040
    custom.bltcon1 = 0x0;       // dff042

    custom.bltapt = source;
    custom.bltdpt = bob->prev_background;

    custom.bltafwm = 0xffff;    // Maschere
    custom.bltalwm = 0xffff;

    custom.bltamod = 40 - (words*2);           // Modulo 0 in A
    custom.bltdmod = 0;

    custom.bltsize = (UWORD) ((rows*5) << 6) | words;
    DisownBlitter();

    bob->prev_background_offset = offset;
}



void draw_bob(BlitterBob* bob,UBYTE* screen, int x,int y) {

    // Se non è la primissima blittata ripristino lo sfondo
    if(!(bob->header.firstdraw)) {
        // TODO: Ripristinare lo sfondo con una bella blittata partendo dagli x e y attuali prima dello
        //       spostamento
        UBYTE* dest_ripristino = screen;
        dest_ripristino += bob->prev_background_offset;
        //printf("Ripristino sfondo con x %d, offset: %d\n",bob->header.x, bob->prev_background_offset);
        simple_blit(bob->prev_background,dest_ripristino,bob->header.words,bob->header.rows,bob->header.bitplanes);
    } else {
        //printf("Primo disegno, nessun ripristino\n");
        bob->header.x = x;
        bob->header.y = y;
    }

    save_background(bob,screen);

    bob->header.x = x;
    bob->header.y = y;
    bob->header.firstdraw = 0;
    
    masked_blit(bob->imgdata,screen,bob->mask,screen,x,y,bob->header.words,bob->header.rows,bob->header.bitplanes);
}

void free_bob(BlitterBob bob) {
    size_t size = (bob.header.words*2)*bob.header.rows*bob.header.bitplanes;
    FreeMem (bob.imgdata,size);
    FreeMem (bob.mask,size);
    FreeMem (bob.prev_background,size);
}

