#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <hardware/dmabits.h>
#include <hardware/custom.h>
#include <stdio.h>
#include "audio.h"
#include "../ptplayer/ptplayer.h"
#include "disk.h"

extern struct Custom custom;

//extern UBYTE mt_Enable;

sound* soundsbuffer[SOUND_SLOTS_SIZE];

UWORD __chip zerosound[32];

// Tengo traccia dello stato dei singoli canali
// 1 se c'è un suono che è stato già avviato e devo fare puntare a valori zero
// il puntatore AUDxLC
UBYTE audiostarted[] = {0,0,0,0};

UWORD audioDmaBits[] = {
    DMAF_AUD0,
    DMAF_AUD1,
    DMAF_AUD2,
    DMAF_AUD3
};

/**
 * @brief Play the sound
 * 
 * Period = 1 / (samplerate * 2,81937 * 10^-7)
 *          
 * @param sound 
 * @param period 
 * @param volume 
 * @param audiochannel 
 */
void playsound(sound* sound,UWORD period,UWORD volume,USHORT audiochannel) {

    if (!mt_Enable) {
        // Setto dmacon senza il bit DMAF_SETCLR, ovvero spengo il bit
        custom.dmacon = audioDmaBits[audiochannel];      // In caso ci sia un suono che sta già suonando e dobbiamo
                                                         // interromperlo
        custom.aud[audiochannel].ac_ptr = sound->sampledata;
        custom.aud[audiochannel].ac_len = sound->size;
        custom.aud[audiochannel].ac_per = period;
        custom.aud[audiochannel].ac_vol = volume;
        custom.dmacon = DMAF_SETCLR | audioDmaBits[audiochannel];    // Via!

        audiostarted[audiochannel] = 1;
    } else {
        SfxStructure pt_sound;
        pt_sound.sfx_ptr = sound->sampledata;
        pt_sound.sfx_len = sound->size;
        pt_sound.sfx_per = period;
        pt_sound.sfx_vol = volume;
        pt_sound.sfx_cha = audiochannel;
        pt_sound.sfx_pri = 127;
        mt_playfx(&custom,&pt_sound);
    }
        
        
}

// Da chiamare alla fine di ogni vblank, magari metterla nell'interrupt
void checkSoundStop() {
    for (int i = 0 ; i < 4; i++) {
        if (audiostarted[i]) {
            custom.aud[i].ac_ptr = zerosound;
            custom.aud[i].ac_len = 16;
            audiostarted[i] = 0;
        }
    }
}

static int find_free_slot() {
    for (int i=0 ; i < SOUND_SLOTS_SIZE; i++) {
        if (soundsbuffer[i] == 0) {
            return i;
        }
    }
    return -1;
}

sound* init_sound(char* filename) {
    sound* s;

    int free_slot = find_free_slot();

    if (free_slot != -1) {
        s = AllocVec(sizeof(sound),MEMF_CHIP|MEMF_CLEAR);

        s->sampledata = alloc_and_load_asset(filename);
        s->size = (findSize(filename))/2;
        soundsbuffer[free_slot] = s;
    }

    return s;
}

void free_sound(sound* s) {
    FreeVec(s->sampledata);
    FreeVec(s);

    for (int i = 0; i < SOUND_SLOTS_SIZE; i++) {
        if (soundsbuffer[i] == s) {
            soundsbuffer[i] = 0;
        }
    }
}

void free_all_sounds() {
    for (int i=0; i < SOUND_SLOTS_SIZE; i++) {
        if (soundsbuffer[i] != 0) {
            free_sound(soundsbuffer[i]);
        }
    }
}