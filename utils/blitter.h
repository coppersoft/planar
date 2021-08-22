#include <clib/exec_protos.h>

void simple_blit(UBYTE* source, UBYTE* dest, int words, int rows, int bitplanes);
void masked_blit(UBYTE* source, UBYTE* dest, UBYTE* mask, UBYTE* background, int x, int y, int words, int rows, int bitplanes);

struct BobHeader {
    WORD    x;
    WORD    y;
    BOOL    firstdraw;
    UBYTE   rows;
    UBYTE   words;
    UBYTE   bitplanes;
};

// Non ho potuto chiamarlo Bob perché è già definito nell'NDK in graphics/gel.h (139)
typedef struct BlitterBob {
    struct  BobHeader header;
    UBYTE*  imgdata;
    UBYTE*  mask;
    UBYTE*  prev_background;
} BlitterBob;

BlitterBob init_bob(char* img_file, char* mask_file, int words, int rows, int bitplanes);
void draw_bob(BlitterBob* bob,UBYTE* dest, int x,int y);
