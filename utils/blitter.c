#include "blitter.h"
#include <clib/graphics_protos.h>
#include <hardware/custom.h>
#include "disk.h"

extern struct Custom custom;
extern BobListElement* bobList;
extern int drawBufferSelector;

/*
    Restituisce quando bisogna spostarsi per il buffer dove scrivere

    0 se il drawBufferSelector è 0
    grandezza singolo framebuffer se è 1
*/
static int getDB_bpls_offset() {
    int bpls_offset = 0;

    if (drawBufferSelector == 1) {
        bpls_offset = (40*256)*5;       // TODO: sistemare
    }
    return bpls_offset;
}


static void addBobToList(BlitterBob* bob) {
    BobListElement* newElement = AllocVec(sizeof(BobListElement),MEMF_CHIP|MEMF_CLEAR);

    newElement->bob = bob;
    newElement->nextBob = 0;
    if (bobList == 0) {
        bobList = newElement;
        return;
    }

    BobListElement* sentry = bobList;

    while (sentry->nextBob != 0) {
        sentry = sentry->nextBob;
    }
    sentry->nextBob = newElement;
    return;
}

static void removeBobFromList(BlitterBob* bob) {
    
    if (bobList != 0) {
        // Caso particolare, è il primo della lista
        if (bobList->bob == bob) {
            BobListElement* secondBobSave = bobList->nextBob; // Mi salvo il puntatore del secondo
            FreeVec(bobList);              // libero la memoria occupata dal primo elemento della lista
            bobList = secondBobSave; // Faccio puntare l'inizio della lista all'elemento successivo
            
        } else {

            BobListElement* actual = bobList;
            while ((actual->nextBob->bob != bob) && actual != 0) {
                actual = actual->nextBob;
            }
            BobListElement* nextBobSave = actual->nextBob->nextBob; // Mi salvo il puntatore a due più avanti
            FreeVec(actual->nextBob);        // Libero la memoria occupata dall'elemento successivo */
            actual->nextBob = nextBobSave; // Faccio puntare l'elemento attuale a due elementi più avanti
            
        }
    } else {
        printf("ATTENZIONE: lista vuota, esco");
        return;
    }
    free_bob(bob);
}




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
}

void masked_blit(UBYTE* source, UBYTE* dest, UBYTE* mask, UBYTE* background, int x, int y, int words, int rows, int bitplanes) {
    
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
}

/**
 * Crea maschera da un bob interleaved
 */
static UBYTE* createMask(unsigned char* bob,int bitplanes, int frames, int rows, int words) {
    size_t size = ((words*2)*rows*bitplanes*frames);

    UBYTE* mask = AllocVec(size,MEMF_CHIP|MEMF_CLEAR);
    UBYTE* work = AllocVec(words*2,MEMF_CHIP|MEMF_CLEAR);

    int mask_idx = 0;
    int bp_idx = 0;
    for (int r = 0; r < rows*frames; r++) {
        // Creo la riga r-esima della maschera
        
        for (int bp = 0; bp < bitplanes; bp++) {
            for (int bt = 0; bt < words*2; bt++) {
                work[bt] |= bob[bp_idx++];
            }
        }
        // Copio la riga nella maschera per tutti i bitplane
        
        for (int i = 0; i < bitplanes; i++) {
            for (int w = 0; w < words*2; w++) {
                mask[mask_idx++] = work[w];
            }
        }

        // Pulisco la maschera di lavoro
        for (int i=0; i < words*2; i++) {
            work[i] = 0;
        }

    }
    FreeVec(work);
    return mask;
}

BlitterBob* init_bob(char* img_file, int words, int rows, int bitplanes, int frames, int x, int y) {
    BlitterBob* newbob = AllocVec(sizeof(BlitterBob),MEMF_CHIP|MEMF_CLEAR);

    newbob->header.bitplanes = bitplanes;
    newbob->header.firstdraw[0] = 1;
    newbob->header.firstdraw[1] = 1;
    newbob->header.rows = rows;
    newbob->header.words = words;
    newbob->header.frames = frames;
    newbob->x = x;
    newbob->y = y;
    newbob->state = DRAW;

    size_t background_size = (words*2)*rows*bitplanes;

    size_t size = background_size*frames;

    newbob->imgdata = alloc_and_load_asset(size,img_file);

    newbob->mask = createMask(newbob->imgdata,newbob->header.bitplanes,newbob->header.frames,newbob->header.rows,newbob->header.words);

    newbob->prev_background[0] = AllocVec(background_size,MEMF_CHIP|MEMF_CLEAR);
    newbob->prev_background[1] = AllocVec(background_size,MEMF_CHIP|MEMF_CLEAR);

    addBobToList(newbob);

    return newbob;
}

/*
    Restituisce di quanti byte ci dobbiamo spostare per il singolo bob
    nel singolo framebuffer

    TODO: Generalizzare il 40!!!
*/
static int getOffset(BlitterBob* bob) {
    int y = bob->y;
    int x = bob->x;
    int bitplanes = bob->header.bitplanes;

    int offset = y*40*bitplanes;
    offset += (x>> 4)*2;
    return offset;
}

void save_background(BlitterBob* bob,UBYTE* source) {
    
    int offset = getOffset(bob);

    source+=offset;

    source+=getDB_bpls_offset();


    int words = bob->header.words;
    int rows = bob->header.rows;
    int bitplanes = bob->header.bitplanes;


    WaitBlit();
    /* 0 = shift nullo
       9 = 1001: abilito solo i canali A e D
       f0 = minterm, copia semplice
    */
    custom.bltcon0 = 0x09f0;    // dff040
    custom.bltcon1 = 0x0;       // dff042

    custom.bltapt = source;
    custom.bltdpt = bob->prev_background[drawBufferSelector];

    custom.bltafwm = 0xffff;    // Maschere
    custom.bltalwm = 0xffff;

    custom.bltamod = 40 - (words*2);           // Modulo 0 in A
    custom.bltdmod = 0;

    custom.bltsize = (UWORD) ((rows*5) << 6) | words;

    bob->prev_background_offset[drawBufferSelector] = offset;
}

void restore_background(BlitterBob* bob,UBYTE* screen) {
    if(!(bob->header.firstdraw[drawBufferSelector])) {
        
        UBYTE* dest_ripristino = screen;
        dest_ripristino += getDB_bpls_offset();
        dest_ripristino += bob->prev_background_offset[drawBufferSelector];
        
        simple_blit(bob->prev_background[drawBufferSelector],
            dest_ripristino,
            bob->header.words,bob->header.rows,bob->header.bitplanes);
    } else {
        //printf("restore_background: prima blittata, non faccio nulla\n");
    }
}

void draw_bob(BlitterBob* bob,UBYTE* screen) {

    bob->header.firstdraw[drawBufferSelector] = 0;

    int frameoffset = (((bob->header.words)*2) * bob->header.rows * bob->header.bitplanes) * bob->frame;
 
    // Offset per l'eventuale double buffer
    int bpls_offset = getDB_bpls_offset();

    masked_blit(bob->imgdata+frameoffset,
                screen+bpls_offset,
                bob->mask+frameoffset,
                screen+bpls_offset,
                bob->x,
                bob->y,
                bob->header.words,
                bob->header.rows,
                bob->header.bitplanes);

    

}

// Fixare dimensione prev_background che è un solo frame
void free_bob(BlitterBob* bob) {
    size_t size_background = (bob->header.words*2)*bob->header.rows*bob->header.bitplanes;
    size_t size = size_background*bob->header.frames;
    FreeVec (bob->imgdata);
    FreeVec (bob->mask);
    FreeVec (bob->prev_background[0]);
    FreeVec (bob->prev_background[1]);
    FreeVec (bob);
}

void draw_bobs(UBYTE* screen) {
    OwnBlitter();
    BobListElement* actual = bobList;

    // Ripristino i vecchi sfondi
    if (actual != 0) {
        do {
            restore_background(actual->bob,screen);

            switch (actual->bob->state) {
                case DRAW: {
                    actual = actual->nextBob;
                    break;
                }
                case TO_BE_DELETED_BUFFER_0: {
                    actual->bob->state = TO_BE_DELETED_BUFFER_1_AND_CLEANED;
                    actual = actual->nextBob;
                    break;
                }
                case TO_BE_DELETED_BUFFER_1_AND_CLEANED: {
                    BlitterBob* toBeRemoved = actual->bob;
                    actual = actual->nextBob;
                    removeBobFromList(toBeRemoved);
                    break;
                }
            }

            
        } while (actual != 0);
    } else {
        return;
    }

    // Salvo lo sfondo attuale
    actual = bobList;
    do {
        if (actual->bob->state == DRAW) {
            save_background(actual->bob,screen);
        }
        actual = actual->nextBob;
    } while (actual != 0);

    actual = bobList;
    do {
        if (actual->bob->state == DRAW) {
            draw_bob(actual->bob,screen);
        }
        actual = actual->nextBob;
    } while (actual != 0);
    DisownBlitter();
}

void remove_bob(BlitterBob* bob) {
    bob->state = TO_BE_DELETED_BUFFER_0;
}