#include <clib/exec_protos.h>

typedef struct Screen {
    UWORD   bytes_width;
    UWORD   height;
    UBYTE   depth;
    UBYTE   left_margin_bytes;
    UBYTE   right_margin_bytes;
    UBYTE   display_start_offset_bytes;
    BOOL    dual_playfield;
    UBYTE*  bitplanes;
    ULONG   framebuffer_size;
} Screen;

