#include "animation.h"
#include "disk.h"
#include <clib/exec_protos.h>

animation* init_animation(char* filename, USHORT anim_type) {

    animation* new_animation = AllocVec(sizeof(animation),MEMF_CLEAR);  // Allowed to stay in fast ram

    new_animation->animdata = alloc_and_load_asset(filename);
    new_animation->size = new_animation->animdata[0];
    new_animation->actualFrame = 0;
    new_animation->type = anim_type;
    new_animation->status = PLAYING;

    return new_animation;
}

void free_animation(animation* animation) {
    FreeVec(animation->animdata);
    FreeVec(animation);
}

