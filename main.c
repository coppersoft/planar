/*

    Planar - A C Game engine for your Amiga

    (C) 2021 - Lorenzo Di Gaetano

*/

//#define JACK_AND_COMPANY
#define BLOCKS

#include <stdio.h>
#include <hardware/custom.h>
#include <hardware/dmabits.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <graphics/gfxbase.h>
#include "planar/ahpc_registers.h"
#include "planar/init.h"
#include "planar/bitplanes.h"
#include "planar/sprites.h"
#include "planar/vblank.h"
#include "planar/blitter.h"
#include "planar/disk.h"
#include "planar/input.h"
#include "planar/debug.h"
#include "planar/screen.h"
#include "planar/audio.h"
#include "planar/interrupt.h"
#include "planar/collision.h"
#include "planar/animation.h"

/*

    https://www.youtube.com/watch?v=zX5q40bDWxI

    Compilare con 

    vc +kick13 -c99 -I$NDK_INC main.c -lamiga -lauto -o main.exe

    make

*/


// 20 instead of 127 because of input.device priority
// Una priorità troppo alta potrebbe impedire a un eventuale input handler di ottenere CPU
// L'input handler del sistema operativo gira a priorità 20, quindi se lo settiamo a 20
// entrambi i task avranno cicli CPU e potremmo continuare a ricevere input da tastiera
#define TASK_PRIORITY           (20)




// Assets sizes in bytes
//#define GRAPHICS_BPLS_SIZE   (40*256)*5
#define BLOCK_SIZE           (2*16*5)
#define EXPLOSION_FRAME_SIZE (6*32*5)   // larghezza 48 px (32 + 16 padding), altezza 32 righe




extern struct GfxBase *GfxBase;
extern struct Custom custom;

static UWORD __chip greenShipRight[] = {
    	0x0000,0x0000,
	0x0000,0x0000,
	0x0080,0x00e0,
	0x00a0,0x00d0,
	0x0090,0x00e8,
	0x0088,0x00f4,
	0x0084,0x40fa,
	0x80ff,0xc0ff,
	0x8000,0xc000,
	0x8000,0xe000,
	0xc000,0xf800,
	0xf000,0xff00,
	0x7e00,0x7ffe,
	0x3ffc,0x3ffc,
	0x0000,0x0000,
	0x0000,0x0080,
	0x03c0,0x03c0,
	0x07e0,0x0760,
	0x0ff0,0x0f70,
	0x1ff8,0x1f78,
	0x3ffc,0x3f7c,
	0x3ffe,0x7f7e,
	0x3fff,0xff00,
	0x3fff,0xffff,
	0x1fff,0xffff,
	0x07ff,0xffff,
	0x00ff,0xffff,
	0x0000,0x7ffe,
	0x0000,0x3ffc,
	0x0000,0x0000
};

static UWORD __chip redShipRight[] = {
		0x0000,0x0000,
	0x0000,0x03c0,
	0x0080,0x07e0,
	0x00a0,0x0fd0,
	0x0090,0x1fe8,
	0x0088,0x3ff4,
	0x0084,0x3ffa,
	0x80ff,0xbfff,
	0x8000,0xbfff,
	0x8000,0x9fff,
	0xc000,0xc7ff,
	0xf000,0xf0ff,
	0x7e00,0x7e00,
	0x3ffc,0x3ffc,
	0x0000,0x0000,      // Fine primo sprite
	0x0000,0x0080,      // word di controllo del secondo sprite (attached)
	0x03c0,0x0000,
	0x07e0,0x0060,
	0x0ff0,0x0070,
	0x1ff8,0x0078,
	0x3ffc,0x007c,
	0x3ffe,0x407e,
	0xbfff,0x4000,
	0xbfff,0x4000,
	0x9fff,0x6000,
	0xc7ff,0x3800,
	0xf0ff,0x0f00,
	0x7e00,0x01fe,
	0x3ffc,0x0000,
	0x0000,0x0000,
};

static UWORD __chip redShipLeft[] = {
	0x0000,0x0000,
	0x0000,0x03c0,
	0x0100,0x07e0,
	0x0500,0x0bf0,
	0x0900,0x17f8,
	0x1100,0x2ffc,
	0x2100,0x5ffc,
	0xff01,0xfffd,
	0x0001,0xfffd,
	0x0001,0xfff9,
	0x0003,0xffe3,
	0x000f,0xff0f,
	0x007e,0x007e,
	0x3ffc,0x3ffc,
	0x0000,0x0000,  // Fine primo sprite
	0x0000,0x0080,  // word di controllo del secondo sprite (attached)
	0x03c0,0x0000,
	0x07e0,0x0600,
	0x0ff0,0x0e00,
	0x1ff8,0x1e00,
	0x3ffc,0x3e00,
	0x7ffc,0x7e02,
	0xfffd,0x0002,
	0xfffd,0x0002,
	0xfff9,0x0006,
	0xffe3,0x001c,
	0xff0f,0x00f0,
	0x007e,0x7f80,
	0x3ffc,0x0000,
	0x0000,0x0000
};

    /*
        TODO: il drawbufferselector non dovrebbe stare qui
    */
    int drawBufferSelector = 0;

int main(int argc, char **argv)
{
    SetTaskPri(FindTask(NULL), TASK_PRIORITY);
    
    custom.dmacon = DMAF_AUD0;

    installVertbInterrupt((APTR)checkSoundStop,NULL);

    /*
        SPRITES
    */

    // Punto gli sprite 0-7 a null
    reset_sprites(&copperlist[SPR0PTH_VALUE_IDX]);

    point_sprite_number(0,redShipRight);
    point_sprite_number(1,&redShipRight[30]);
    
    point_sprite_number(2,greenShipRight);
    point_sprite_number(3,&greenShipRight[30]);
    

    WORD ship_x = 125, ship_y = 0, ship_height = 13;
    set_sprite_pos(redShipRight, ship_x, ship_y, ship_height);
    set_sprite_pos(&redShipRight[30], ship_x, ship_y, ship_height);

    init_screen(40,256,8,0,0,0,1);

    


    load_asset(screen.bitplanes, "pulsar.raw");

    

    load_asset(screen.bitplanes + screen.framebuffer_size,"pulsar.raw");
    
    /* load_asset(screen.bitplanes + (screen.framebuffer_size / 2),
                40*256*4,
                "ColoriPF2.raw");

    load_asset(screen.bitplanes + (screen.framebuffer_size / 2) + screen.framebuffer_size,
                40*256*4,
                "ColoriPF2.raw"); */

    
    UWORD bplcon2 = custom.bplcon2;
    bplcon2 |= 0x40;     // Playfield 2 priorità su playfield 1 (ci sta sopra)
    bplcon2 |= 0x24;     // Priorità massima agli sprite su tutti e 2 i Playfield

    custom.bplcon2 = bplcon2;

    UWORD bplcon4 = custom.bplcon4;

    custom.bplcon4 = 0x22;  // Seleziono la palette degli sprite

    wait_vblank();

    sound* bang = init_sound("bang.raw");
    sound* udeath = init_sound("udeath.raw");

    BlitterBob* block1 = init_bob("16block.raw",2,16,4,1,0,0,BOBTYPE_FAST,PLAYFIELD_2);
    BlitterBob* block2 = init_bob("16block_stripe.raw",2,16,4,1,0,0,BOBTYPE_FAST,PLAYFIELD_2);
    BlitterBob* block3 = init_bob("16block.raw",2,16,4,1,0,0,BOBTYPE_FAST,PLAYFIELD_2);
    BlitterBob* block4 = init_bob("16block_stripe.raw",2,16,4,1,0,0,BOBTYPE_FAST,PLAYFIELD_2);
    BlitterBob* block5 = init_bob("16block.raw",2,16,4,1,0,0,BOBTYPE_FAST,PLAYFIELD_2);
    BlitterBob* block6 = init_bob("16block_stripe.raw",2,16,4,1,0,0,BOBTYPE_FAST,PLAYFIELD_2);

    BlitterBob* block7 = init_bob("16block.raw",2,16,4,1,0,0,BOBTYPE_FAST,PLAYFIELD_2);

    BlitterBob* block8 = init_bob("16block_stripe.raw",2,16,4,1,0,0,BOBTYPE_FAST,PLAYFIELD_2);

    /*
        Test animation
    */

    animation* anim_block8 = init_animation("movement1.pa",CYCLIC);
    //printf("Animation size %d\n",anim_block8->size);

    int frame = 5;

    UWORD x = anim_block8->animdata[(frame*2)+1];
    UWORD y = anim_block8->animdata[(frame*2)+2];

    //printf("x %d\n",x);
    //printf("y %d\n",y);

    block8->animation = anim_block8;

    block1->x = 10;
    block1->y = 10;
    block2->x = 40;
    block2->y = 40;
    block3->x = 60;
    block3->y = 60;
    block4->x = 80;
    block4->y = 80;
    block5->x = 100;
    block5->y = 100;
    block6->x = 120;
    block6->y = 120;

    block7->y = 70;
    block7->x = 70;

    block8->x = 10;
    block8->y = 10;

    // sprite giocatore 1
    int orientamento1 = 0;
    int spostamentox = 0;
    int spostamentoy = 0;

    int conta = 0;

    int shipVerde_x = 50;
    int shipVerde_y = 50;

    while(!isMousePressed()) {

        conta++;

        /*
            INIZIO GESTIONE TEST CON BLOCK 7

        */



        if (conta > 10) {
            disable_bob(block7);
        }

        if (conta > 20) {
            enable_bob(block7);
            conta = 0;
        }

        if (checkJoystick_horiz(JOY1)) {
            if (checkJoystick_horiz(JOY1) == JOY_LEFT) {
                block7->x--;
            }
            if (checkJoystick_horiz(JOY1) == JOY_RIGHT) {
                block7->x++;
            }
        } 
        
        if (checkJoystick_vert(JOY1)) {
            if (checkJoystick_vert(JOY1) == JOY_UP) {
                block7->y--;
            }
            if (checkJoystick_vert(JOY1) == JOY_DOWN) {
                block7->y++;
            }
        } 


        // Controllo se il block7 è in collisione con il block3

        if (checkCollision(block7,block3)) {
            custom.color[0] = 0xf000;
            custom.color[1] = 0xf000;
            custom.color[2] = 0xf000;
            custom.color[3] = 0xf000;
            custom.color[4] = 0xf000;
            custom.color[5] = 0xf000;
            custom.color[6] = 0xf000;
            custom.color[7] = 0xf000;
            custom.color[8] = 0xf000;
            custom.color[9] = 0xf000;
            custom.color[10] = 0xf000;

            playsound(udeath,310,63,AUD2);
        }

        /*
            FINE GESTIONE TEST CON BLOCK 7
        */




        block1->x++;
        if (block1->x > 320-16) {
            block1->x = 0;
        }
        
        block2->x++;
        if (block2->x > 320-16) {
            block2->x = 0;
        }

        block3->x++;
        if (block3->x > 320-16) {
            block3->x = 0;
        }

        block4->x++;
        if (block4->x > 320-16) {
            block4->x = 0;
        }

        block5->x++;
        if (block5->x > 320-16) {
            block5->x = 0;
        }

        block6->x++;
        if (block6->x > 320-16) {
            block6->x = 0;
        }

        if (checkJoystick_horiz(JOY1)) {
            if (checkJoystick_horiz(JOY1) == JOY_LEFT) {
                spostamentox--;
                orientamento1 = 0;
            }
            if (checkJoystick_horiz(JOY1) == JOY_RIGHT) {
                spostamentox++;
                orientamento1 = 1;
            }
        } else {
            if (spostamentox > 0) {
                spostamentox--;
            } else if (spostamentox < 0) {
                spostamentox++;
            }
        }
        
        if (checkJoystick_vert(JOY1)) {
            if (checkJoystick_vert(JOY1) == JOY_UP) {
                spostamentoy -= 1;
            }
            if (checkJoystick_vert(JOY1) == JOY_DOWN) {
                spostamentoy += 1;
            }
        } else {
            if (spostamentoy > 0) {
                spostamentoy--;
            } else if (spostamentoy < 0) {
                spostamentoy++;
            }
        }

        // Gestione accelerazione massima
        if (spostamentox == 9) {
            spostamentox = 8;
        }
        if (spostamentoy == 9) {
            spostamentoy = 8;
        }
        if (spostamentox == -9) {
            spostamentox = -8;
        }
        if (spostamentoy == -9) {
            spostamentoy = -8;
        }

        ship_x += spostamentox/2;
        ship_y += spostamentoy/2;

        // Gestione margini di schermo
        if (ship_x < 0) {
            ship_x = 0;
            spostamentox=0;
        }
        if (ship_x > 320-16) {
            ship_x = 320-16;
            spostamentox=0;
        }
        if (ship_y < 0) {
            ship_y = 0;
            spostamentoy=0;
        }
        if (ship_y > 256-14) {
            ship_y = 256-14;
            spostamentoy = 0;
        }

        if (orientamento1 == 0) {
            set_sprite_pos(redShipLeft, ship_x, ship_y, ship_height);
            set_sprite_pos(&redShipLeft[30], ship_x, ship_y, ship_height);
            point_sprite_number(0,redShipLeft);
            point_sprite_number(1,&redShipLeft[30]);
        } else {
            set_sprite_pos(redShipRight, ship_x, ship_y, ship_height);
            set_sprite_pos(&redShipRight[30], ship_x, ship_y, ship_height);
            point_sprite_number(0,redShipRight);
            point_sprite_number(1,&redShipRight[30]);
        }
        
        // Prova collisione sprite / playfield
        if (custom.clxdat & 0x20) {     // Collisione
            //set_sprite_pos(greenShipRight, 320-16, 0, ship_height);
            //set_sprite_pos(&greenShipRight[30], 320-16, 0, ship_height);
            playsound(bang,160,63,AUD1);
        } else {
            //set_sprite_pos(greenShipRight, 0, 0, ship_height);
            //set_sprite_pos(&greenShipRight[30], 0, 0, ship_height);
        }


        set_sprite_pos(greenShipRight, shipVerde_x, shipVerde_y, ship_height);
        set_sprite_pos(&greenShipRight[30], shipVerde_x, shipVerde_y, ship_height);

        shipVerde_x += getMouse0HorizMovement();
        shipVerde_y += getMouse0VertMovement();

        draw_bobs();

        switchBuffers();

        wait_vblank();
        

    }

    custom.dmacon = DMAF_AUD0 | DMAF_AUD1 | DMAF_AUD2 | DMAF_AUD3;


    //waitmouse();
    
    uninstallVertbInterrupt();

    free_bitplanes(screen.bitplanes);

    free_all_bobs();

    free_all_sounds();

    free_animation(anim_block8);
   
    reset_display();

    

    
    return 0;
}