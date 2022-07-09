#ifndef _INPUT_
#define _INPUT_

#include <clib/exec_protos.h>

#define PRA_FIR0_BIT            (1 << 6)
#define PRA_FIR1_BIT            (1 << 7)

enum horiz {
    JOY_LEFT = 1,
    JOY_RIGHT = 2
};

enum vert {
    JOY_UP = 1,
    JOY_DOWN = 2
};

enum joystick {
    JOY0,
    JOY1
};

enum mouse {
    MOUSE0,
    MOUSE1
};

void waitfire();
BOOL isMousePressed();      // TODO, anche per l'altro mouse, anche se non si usa praticamente mai
void waitmouse();
USHORT checkJoystick_horiz(USHORT joystick);
USHORT checkJoystick_vert(USHORT joystick);
BYTE getMouse0VertMovement();
BYTE getMouse0HorizMovement();

#endif
