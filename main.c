/*

    Sensitive for Amiga

    (C) 2021 - Lorenzo Di Gaetano

    This game is a port of the legendary Commodore 64 game "Sensitive" by Oliver Kirwa

*/

#include <stdio.h>
#include <hardware/custom.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <graphics/gfxbase.h>
#include "ahpc_registers.h"

/*

    https://www.youtube.com/watch?v=zX5q40bDWxI

    Compilare con 

    vc +kick13 -c99 -I$NDK_INC main.c -lamiga -lauto -o main.exe

*/


// 20 instead of 127 because of input.device priority
// Una priorità troppo alta potrebbe impedire a un eventuale input handler di ottenere CPU
// L'input handler del sistema operativo gira a priorità 20, quindi se lo settiamo a 20
// entrambi i task avranno cicli CPU e potremmo continuare a ricevere input da tastiera
#define TASK_PRIORITY           (20)
#define PRA_FIR0_BIT            (1 << 6)

// 5 bitplanes, composite color.
#define BPLCON0_5BPP_COMPOSITE_COLOR 0x5200

#define GRAPHICS_BPLS_SIZE   (40*256)*5

// Diwstart e stop
#define DIWSTRT_VALUE      0x2c81
#define DIWSTOP_VALUE_PAL  0x2cc1

// Data fetch
#define DDFSTRT_VALUE      0x0038
#define DDFSTOP_VALUE      0x00d0

// copper instruction macros
#define COP_MOVE(addr, data) addr, data
#define COP_WAIT_END  0xffff, 0xfffe

// Indici array copperlist
#define BPL1PTH_VALUE_IDX (17)

extern struct GfxBase *GfxBase;
extern struct Custom custom;

/*
    5:00
    __chip dice a vbcc di mettere la roba in chip, ovviamente per la coppelist in questo caso

    "la prima move dice al computer di emettere nello schermo i colori registrati nel color register zero
    lo facciamo settando il color burst bit in BPLCON0"

    http://amiga-dev.wikidot.com/hardware:bplcon0  (bit 9, infatti vedi BPLCON0_COMPOSITE_COLOR)

*/
static UWORD __chip copperlist[] = {

    COP_MOVE(FMODE,   0), // set fetch mode = 0 (slow fetch mode for AGA compatibility)
    COP_MOVE(DDFSTRT, DDFSTRT_VALUE),
    COP_MOVE(DDFSTOP, DDFSTOP_VALUE),
    COP_MOVE(DIWSTRT, DIWSTRT_VALUE),
    COP_MOVE(DIWSTOP, DIWSTOP_VALUE_PAL),
    COP_MOVE(BPLCON0, BPLCON0_5BPP_COMPOSITE_COLOR),

    // Bitplane modulos, interleaved, quindi 40 byte * 5-1 bitplane
    COP_MOVE(BPL1MOD, 40*4),
    COP_MOVE(BPL2MOD, 40*4),
    // Bitplane pointers.
    COP_MOVE(BPL1PTH, 0),
    COP_MOVE(BPL1PTL, 0),
    COP_MOVE(BPL2PTH, 0),
    COP_MOVE(BPL2PTL, 0),
    COP_MOVE(BPL3PTH, 0),
    COP_MOVE(BPL3PTL, 0),
    COP_MOVE(BPL4PTH, 0),
    COP_MOVE(BPL4PTL, 0),
    COP_MOVE(BPL5PTH, 0),
    COP_MOVE(BPL5PTL, 0),

// Palette
	0x0180,0x0000,0x0182,0x0aaa,0x0184,0x0e00,0x0186,0x0a00,
	0x0188,0x0d80,0x018a,0x0fe0,0x018c,0x08f0,0x018e,0x0080,
	0x0190,0x00b6,0x0192,0x00dd,0x0194,0x00af,0x0196,0x007c,
	0x0198,0x000f,0x019a,0x070f,0x019c,0x0c0e,0x019e,0x0c08,
	0x01a0,0x0620,0x01a2,0x0e52,0x01a4,0x0a52,0x01a6,0x0fca,
	0x01a8,0x0333,0x01aa,0x0444,0x01ac,0x0555,0x01ae,0x0666,
	0x01b0,0x0777,0x01b2,0x0888,0x01b4,0x0999,0x01b6,0x0aaa,
	0x01b8,0x0ccc,0x01ba,0x0ddd,0x01bc,0x0eee,0x01be,0x0fff,

/*
    COP_MOVE(COLOR00, 0x000),
    0x7c07, 0xfffe,            // wait for 1/3 (0x07, 0x7c)
    COP_MOVE(COLOR00, 0xf00),
    0xda07, 0xfffe,            // wait for 2/3 (0x07, 0xda)
    COP_MOVE(COLOR00, 0xff0),
*/
    COP_WAIT_END
};

/*
    3:00
    Questa funzione rende il display disponibile al nostro programma e determina se siamo
    in PAL o NTSC.

    * LoadView setta i registri hardware relativi al display a una vista vuota
        http://amigadev.elowar.com/read/ADCD_2.1/Includes_and_Autodocs_2._guide/node0459.html
    * WaitTOF si assicura che il copper ha finito di eseguire la copperlist precedente prima che noi
       installiamo la nostra copperlist. Viene chiamata due volte perché ci sono casi in cui ci sono
       due copperlist
    * L'ultima istruzione serve per vedere se siamo in un sistema PAL o NTSC. Controlla i display flag
      nella struttura base della graphics.library.

    NOTA BENE: Qui non stiamo aprendo esplicitamente la graphics.library (come faremmo in asm)
               vbcc e pochi altri compilatori C per l'amiga possono aprire e chiudere automaticamente
               molte librerie standard semplicemente usando lo switch auto (il -lauto nella fase di
               compilazione).
            
               Quindi è sufficiente dichiarare il puntatore alla graphics.library con

               extern struct GfxBase *GfxBase; (vedi sopra)

               e verrà generato da vbcc e usato per creare i display flags.
*/
BOOL init_display(void)
{
    LoadView(NULL);  // clear display, reset hardware registers
    WaitTOF();       // 2 WaitTOFs to wait for 1. long frame and
    WaitTOF();       // 2. short frame copper lists to finish (if interlaced)
    return (((struct GfxBase *) GfxBase)->DisplayFlags & PAL) == PAL;
}

/*
    La prima chiamata è a LoadView, passandogli l'ultima active view memorizzata
    nella graphics library base. Questo rinizializza i registri relativi al display alla
    actiview precedente

    (GfxBase.ActiView è un oggetto View, definito in graphics/view.h)

    I due WaitTOF aspettano il copper che finisce

    Poi settiamo la copperlist a quella del workbench

    Infine RethinkDisplay ricostruisce il display del workbench
    http://amigadev.elowar.com/read/ADCD_2.1/Includes_and_Autodocs_2._guide/node024B.html
    
*/
void reset_display(void)
{
    LoadView(((struct GfxBase *) GfxBase)->ActiView);
    WaitTOF();
    WaitTOF();
    custom.cop1lc = (ULONG) ((struct GfxBase *) GfxBase)->copinit;
    RethinkDisplay();
}

void waitmouse(void)
{
    volatile UBYTE *ciaa_pra = (volatile UBYTE *) 0xbfe001;
    while ((*ciaa_pra & PRA_FIR0_BIT) != 0) ;
}

/*
    Funzione generica di puntamento bitplane

    UBYTE* bitplanes    -> Il puntatore ai bitplane
    UWORD* BPL1PTH_addr -> Puntatore al valore in coppertlist di BPL1PTH
    int    bpl_number   -> Numero bitplane da puntare
*/
void point_bitplanes (UBYTE* bitplanes, UWORD* BPL1PTH_addr, int bpl_number) {
    int coplist_idx = BPL1PTH_VALUE_IDX;
    ULONG addr;
    int bplptr_offset = 0;
    for (int i = 0; i < bpl_number; i++) {
            addr = (ULONG) &(bitplanes[i * 40]);
            BPL1PTH_addr[bplptr_offset] = (addr >> 16) & 0xffff;
            BPL1PTH_addr[bplptr_offset + 2] = addr & 0xffff;
            bplptr_offset += 4; // next bitplane
    }
}



int main(int argc, char **argv)
{
    SetTaskPri(FindTask(NULL), TASK_PRIORITY);
    BOOL is_pal = init_display();
    printf("PAL display: %d\n", is_pal);

    /*
        Prendo un blocco di memoria libero in CHIP e lo pulisco con AllocMem

        http://amigadev.elowar.com/read/ADCD_2.1/Includes_and_Autodocs_2._guide/node024B.html
    */
    UBYTE   *bitplanes;
    bitplanes = AllocMem(GRAPHICS_BPLS_SIZE,MEMF_CHIP|MEMF_CLEAR);

    point_bitplanes(bitplanes,&copperlist[BPL1PTH_VALUE_IDX],5);

    /*for (int i = 0; i < 40*256; i++) {
        bitplanes[i] = 123;
    }*/

    /*
        Leggo il file nella memoria allocata
    */
    FILE *fp = fopen("Pic.raw", "rb");
    if (fp) {
        fread(bitplanes, sizeof(unsigned char), 51200, fp);
    } else {
        printf("ratr0_read_tilesheet() error: file '%s' not found\n", "Pic.raw");
        return 0;
    }

    /*
        Settiamo il puntatore alla copperlist1, usiamo anche qui la variabile "custom" che
        il compilatore ci fornisce per accedere ai registri custom
    */
    custom.cop1lc = (ULONG) copperlist;
    waitmouse();  // replace with logic
    reset_display();
    FreeMem(bitplanes,GRAPHICS_BPLS_SIZE);
    return 0;
}