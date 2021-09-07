#include <clib/exec_protos.h>

void simple_blit(UBYTE* source, UBYTE* dest, int words, int rows, int bitplanes);
void masked_blit(UBYTE* source, UBYTE* dest, UBYTE* mask, UBYTE* background, int x, int y, int words, int rows, int bitplanes);

enum bobState {DRAW,
                TO_BE_DELETED_BUFFER_0,
                TO_BE_DELETED_BUFFER_1_AND_CLEANED};

struct BobHeader {
    BOOL    firstdraw[2];
    UBYTE   rows;
    UBYTE   words;
    UBYTE   bitplanes;
    int     frames;
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

} BlitterBob;

typedef struct BobListElement {
    BlitterBob*                         bob;
    struct BobListElement*       nextBob;
} BobListElement;

BlitterBob* init_bob(char* img_file, int words, int rows, int bitplanes, int frames, int x, int y);
void draw_bob(BlitterBob* bob,UBYTE* screen);
void free_bob(BlitterBob* bob);
void restore_background(BlitterBob* bob,UBYTE* screen);
void clean_both_backgrounds(BlitterBob* bob,UBYTE* screen);
void save_background(BlitterBob* bob,UBYTE* source);
void draw_bobs(UBYTE* screen);
void remove_bob(BlitterBob* bob);
