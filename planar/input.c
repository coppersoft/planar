#include "input.h"
#include <hardware/custom.h>

extern struct Custom custom;

BYTE joy0_horiz_prec = 0;
BYTE joy0_vert_prec = 0;

BYTE joy1_horiz_prec = 0;
BYTE joy1_vert_prec = 0;

void waitfire() {
    volatile UBYTE *ciaa_pra = (volatile UBYTE *) 0xbfe001;
    while ((*ciaa_pra & PRA_FIR1_BIT) != 0) ;
}

BOOL isMousePressed() {
    volatile UBYTE *ciaa_pra = (volatile UBYTE *) 0xbfe001;
    
    if ((*ciaa_pra & PRA_FIR0_BIT) == 0) {
        return 1;
    } else {
        return 0;
    }
}

void waitmouse()
{
    volatile UBYTE *ciaa_pra = (volatile UBYTE *) 0xbfe001;
    while ((*ciaa_pra & PRA_FIR0_BIT) != 0) ;
}

// Vedi http://amiga-dev.wikidot.com/hardware:joy0dat
USHORT checkJoystick_horiz(USHORT joystick) {

    UWORD joydat;

    if (joystick == JOY0) {
        joydat = custom.joy0dat;
    } else {
        joydat = custom.joy1dat;
    }
    
    if (joydat & 0x200){     // Sinistra? Ovvero bit 9 è true
        return JOY_LEFT;
    }
    if (joydat & 2) {         // destra? Ovvero bit 1 è true 
        return JOY_RIGHT;
    }
    return 0;
}

USHORT checkJoystick_vert(USHORT joystick) {

    UWORD joydat;

    if (joystick == JOY0) {
        joydat = custom.joy0dat;
    } else {
        joydat = custom.joy1dat;
    }

    UWORD Y1 = (joydat >> 9) & 1;
    UWORD Y0 = (joydat >> 8) & 1;
    if (Y1 ^ Y0) {      // SU?
        return JOY_UP;
    }

    UWORD X1 = (joydat >> 1) & 1;
    UWORD X0 = joydat & 1;
    if (X1 ^ X0) {      // GIU
        return JOY_DOWN;
    }
    return 0;
}

BYTE getMouse0VertMovement() {
    volatile BYTE *joy0dat_h = (volatile BYTE *) 0xdff00a;  // posizione verticale

    BYTE offset = 0;

    if (joy0_vert_prec == 0) {
        offset = 0;
    } else {
        offset = *joy0dat_h - joy0_vert_prec;
    }
    joy0_vert_prec = *joy0dat_h;
    return offset;
}

BYTE getMouse0HorizMovement() {
    volatile BYTE *joy0dat_l = (volatile BYTE *) 0xdff00b;  // posizione orizzontale

    BYTE offset = 0;

    if (joy0_horiz_prec == 0) {
        offset = 0;
    } else {
        offset = *joy0dat_l - joy0_horiz_prec;
    }
    joy0_horiz_prec = *joy0dat_l;
    return offset;
}

