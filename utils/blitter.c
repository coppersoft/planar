#include "blitter.h"
#include <clib/graphics_protos.h>
#include <hardware/custom.h>
#include "disk.h"

extern struct Custom custom;
extern BobListElement* bobList;

extern BOOL doublebuffer;
extern int drawBufferSelector;

static void addBobToList(BlitterBob* bob) {
    BobListElement* newElement = AllocMem(sizeof(BobListElement),MEMF_CHIP|MEMF_CLEAR);

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
            FreeMem(bobList,sizeof(BobListElement));              // libero la memoria occupata dal primo elemento della lista
            bobList = secondBobSave; // Faccio puntare l'inizio della lista all'elemento successivo
            
        } else {

            BobListElement* actual = bobList;
            while ((actual->nextBob->bob != bob) && actual != 0) {
                actual = actual->nextBob;
            }
            BobListElement* nextBobSave = actual->nextBob->nextBob; // Mi salvo il puntatore a due più avanti
            FreeMem(actual->nextBob,sizeof(BobListElement));        // Libero la memoria occupata dall'elemento successivo */
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

/**
 * Crea maschera da un bob interleaved
 */
static UBYTE* createMask(unsigned char* bob,int bitplanes, int frames, int rows, int words) {
    size_t size = ((words*2)*rows*bitplanes*frames);

    UBYTE* mask = AllocMem(size,MEMF_CHIP|MEMF_CLEAR);
    UBYTE* work = AllocMem(words*2,MEMF_CHIP|MEMF_CLEAR);

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
    FreeMem(work,words*2);
    return mask;
}

BlitterBob* init_bob(char* img_file, int words, int rows, int bitplanes, int frames, int x, int y) {
    BlitterBob* newbob = AllocMem(sizeof(BlitterBob),MEMF_CHIP|MEMF_CLEAR);

    newbob->header.bitplanes = bitplanes;
    newbob->header.firstdraw = 1;
    newbob->header.rows = rows;
    newbob->header.words = words;
    newbob->header.frames = frames;
    newbob->x = x;
    newbob->y = y;

    size_t background_size = (words*2)*rows*bitplanes;

    if (doublebuffer) {
        background_size *= 2;
    }

    size_t size = background_size*frames;

    newbob->imgdata = alloc_and_load_asset(size,img_file);
    //newbob.mask = alloc_and_load_asset(size,mask_file);

    newbob->mask = createMask(newbob->imgdata,newbob->header.bitplanes,newbob->header.frames,newbob->header.rows,newbob->header.words);

    //newbob->mask = createMask(newbob->imgdata,newbob->header.bitplanes,newbob->header.frames,newbob->header.rows,newbob->header.words);
    newbob->prev_background = AllocMem(background_size,MEMF_CHIP|MEMF_CLEAR);

    addBobToList(newbob);

    return newbob;
}

/*
    TODO: Generalizzare il 40!!!
*/
static int getOffset(BlitterBob* bob) {
    int y = bob->y;
    int x = bob->x;
    int bitplanes = bob->header.bitplanes;

    //printf("getoffset: x: %d - y: %d - bpl: %d\n",x,y,bitplanes);

    int offset = y*40*bitplanes;
    offset += (x>> 4)*2;
    //printf("Valore calcolato dentro getoffset %d\n",offset);
    return offset;
}

void save_background(BlitterBob* bob,UBYTE* source) {
    
    int offset = getOffset(bob);

    source+=offset;

    //printf("save_background, offset %d\n",getOffset(bob));

    int words = bob->header.words;
    int rows = bob->header.rows;
    int bitplanes = bob->header.bitplanes;

/*
    ////printf("save_background \n");
    ////printf("words: %d\n",words);
    ////printf("rows: %d\n",rows);
    ////printf("bitplanes: %d\n",bitplanes);
    ////printf("offset: %d\n",getOffset(bob));
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

void restore_background(BlitterBob* bob,UBYTE* screen) {
    if(!(bob->header.firstdraw)) {
        ////printf("restore_background: ripristino\n");
        // TODO: Ripristinare lo sfondo con una bella blittata partendo dagli x e y attuali prima dello
        //       spostamento
        UBYTE* dest_ripristino = screen;
        dest_ripristino += bob->prev_background_offset;
        ////printf("Ripristino sfondo con x %d, offset: %d\n",bob->header.x, bob->prev_background_offset);
        simple_blit(bob->prev_background,dest_ripristino,bob->header.words,bob->header.rows,bob->header.bitplanes);
    } else {
        //printf("restore_background: prima blittata, non faccio nulla\n");
    }
}

static int getDB_bpls_offset() {
    int bpls_offset = 0;

    if (doublebuffer) {
        if (drawBufferSelector == 1) {
            bpls_offset = (40*256)*5;       // TODO: sistemare
        }
    }
    return bpls_offset;
}

void draw_bob(BlitterBob* bob,UBYTE* screen) {

    bob->header.firstdraw = 0;

    int frameoffset = (((bob->header.words)*2) * bob->header.rows * bob->header.bitplanes) * bob->frame;
 /*    
    printf ("bob imagedata %p\n",bob->imgdata);
    printf ("frameoffset: %d\n",frameoffset);
    printf ("bob imagedata + frameoffset %p\n",bob->imgdata+frameoffset); */

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
    FreeMem (bob->imgdata,size);
    FreeMem (bob->mask,size);
    if (doublebuffer) {
        size_background *= 2;
    }
    FreeMem (bob->prev_background,size_background);
    FreeMem (bob,sizeof(BlitterBob));
}

void draw_bobs(UBYTE* screen) {
    BobListElement* actual = bobList;

    // Ripristino i vecchi sfondi
    if (actual != 0) {
        do {
            restore_background(actual->bob,screen);
            actual = actual->nextBob;
        } while (actual != 0);
    } else {
        return;
    }

    // Salvo lo sfondo attuale
    actual = bobList;
    do {
        save_background(actual->bob,screen);
        actual = actual->nextBob;
    } while (actual != 0);

    actual = bobList;
    do {
        draw_bob(actual->bob,screen);
        actual = actual->nextBob;
    } while (actual != 0);
}

/*
    ATTENZIONE: Così sembra funzionare, ma flickera e ogni tanto "riappare lo sfondo" nel bob cancellato
    anche se in realtà lo fa anche con quelli non cancellati.
    
    Tanto devo implementare il double buffer, quindi probabilmente questo problema si elimina
    spontaneamente.

    Eventualmente, implementare la cancellazione
    come segue

    - Il remove_bob non cancella fisicamente il bob, ma lo marca per essere cancellato al prossimo
      draw_bobs

    - Il draw_bobs, dopo aver ripristinato lo sfondo di tutti i bob, se il bob è marcato per essere
      cancellato, non ne salva lo sfondo e lo cancella dalla lista alla fine di tutto i cicli
      (o magari al volo dopo aver salvato il puntatore al successivo bob)

*/
void remove_bob(BlitterBob* bob, UBYTE* screen) {
    restore_background(bob,screen);
    removeBobFromList(bob);
}