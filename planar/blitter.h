#ifndef _BLITTER_
#define _BLITTER_

#include <clib/exec_protos.h>
#include "animation.h"


void simple_blit(UBYTE* source, UBYTE* dest, int words, int rows, int bitplanes);
void masked_blit(UBYTE* source, UBYTE* dest, UBYTE* mask, UBYTE* background, int x, int y, int words, int rows, int bitplanes);

/**
 * @brief definisce l'area di collisione del bob partendo dalla x e y del bob,
 *        di default xoffset e yoffset sono 0
 *        width è (words-1)*16
 */
typedef struct {
    USHORT xoffset;
    USHORT yoffset;
    USHORT width;
    USHORT height; 
} colBound;

enum bobState {DRAW,
                TO_BE_DELETED_BUFFER_0,
                TO_BE_DELETED_BUFFER_1_AND_CLEANED,
                TO_BE_DISABLED_BUFFER_0,
                TO_BE_DISABLED_BUFFER_1,
                DISABLED};

/*
    BOBTYPE_FAST - Background is zeroed before Bob blitting
    BOBTYPE_FULL - Background is saved and restored before Bob blitting
*/
enum bobType {
    BOBTYPE_FAST,
    BOBTYPE_FULL
};

enum playfield {
    PLAYFIELD_1,
    PLAYFIELD_2
};

struct BobHeader {
    BOOL    firstdraw[2];
    UBYTE   rows;
    UBYTE   words;
    UBYTE   bitplanes;
    ULONG   framesize;
    int     frames;
    int     bobType;
    int     playfield;
    colBound    colBound;
};

// Non ho potuto chiamarlo Bob perché è già definito nell'NDK in graphics/gel.h (139)
typedef struct BlitterBob {
    
    struct  BobHeader header;
    WORD    x;
    WORD    y;
    UBYTE*  imgdata;
    UBYTE*  mask;
    UBYTE*  prev_background[2];
    int     prev_background_offset[2];
    int     frame;
    int     state;              // 0  not to be deleted
                                // 1  delete first drawbufferbackground
                                // 2  delete second drawBufferBackground and free
    animation* animation;
} BlitterBob;

typedef struct BobListElement {
    BlitterBob*                         bob;
    struct BobListElement*       nextBob;
} BobListElement;

BlitterBob* init_bob(char* img_file, int words, int rows, int bitplanes, int frames, int x, int y, int bobType, int playfield);
void draw_bob(BlitterBob* bob,UBYTE* screen);
void free_bob(BlitterBob* bob);
void restore_background(BlitterBob* bob,UBYTE* screen);
void save_background(BlitterBob* bob,UBYTE* source);
void draw_bobs();
void remove_bob(BlitterBob* bob);
void disable_bob(BlitterBob* bob);
void enable_bob(BlitterBob* bob);
void free_all_bobs();
void clear(UBYTE* dest, int words, int rows, int bitplanes);


#endif
