#include "vblank.h"

// 303 è PAL, se NTSC 262
void wait_vblank()
{
    while (((*custom_vposr) & 0x1ff00) != (303<<8)) ;
}