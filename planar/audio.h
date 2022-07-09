#include <clib/exec_protos.h>

#define SOUND_SLOTS_SIZE     (10)

typedef struct sound {
    UWORD   size;
    UWORD*  sampledata;
} sound;

sound* init_sound(char* filename);

enum audiochannels {
    AUD0,
    AUD1,
    AUD2,
    AUD3
};

void checkSoundStop();

void playsound(sound* sound,UWORD period,UWORD volume,USHORT audiochannel);

void free_sound(sound* s);

void free_all_sounds();
