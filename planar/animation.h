#ifndef _ANIMATION_
#define _ANIMATION_

#include <clib/exec_protos.h>


enum ANIM_TYPE {
    CYCLIC,
    BOUNCE,
    ONESHOT
};

enum ANIM_STATUS {
    PLAYING,
    PAUSED,
    END
};

typedef struct animation {
    UWORD*          animdata;
    UWORD           size;
    UWORD           actualFrame;
    USHORT          type;
    USHORT          status;
} animation;

animation* init_animation(char* filename, USHORT anim_type);
void free_animation(animation* animation);
#endif
