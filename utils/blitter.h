#include <clib/exec_protos.h>

void simple_blit(UBYTE* source, UBYTE* dest, int words, int rows, int bitplanes);
void masked_blit(UBYTE* source, UBYTE* dest, UBYTE* mask, UBYTE* background, int x, int y, int words, int rows, int bitplanes);

struct BobHeader {
    WORD    x;
    WORD    y;
    BOOL    firstdraw;
    UBYTE   height;
    UBYTE   width;
    char*   filename;
    char*   mask_filename;
};

// Non ho potuto chiamarlo Bob perché è già definito nell'NKD in graphics/gel.h (139)
struct BlitterBob {
    struct  BobHeader header;
    UBYTE*  imgdata;
    UBYTE*  mask;
    UBYTE*  background;
};
