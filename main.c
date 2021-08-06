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
#define COLOR00                 (0x180)
#define BPLCON0                 (0x100)
#define PRA_FIR0_BIT            (1 << 6)
#define BPLCON0_COMPOSITE_COLOR (1 << 9)

// copper instruction macros
#define COP_MOVE(addr, data) addr, data
#define COP_WAIT_END  0xffff, 0xfffe

extern struct GfxBase *GfxBase;
extern struct Custom custom;

/*
    5:00
    __chip dice a vbcc di mettere la roba in chip, ovviamente per la coppelist in questo caso

    "la prima move dice al computer di emettere nello schermo i colori registrati nel color register zero
    lo facciamo settando il color burst bit in BPLCON0"

    http://amiga-dev.wikidot.com/hardware:bplcon0  (bit 9, infatti vedi BPLCON0_COMPOSITE_COLOR)

    


*/
static UWORD __chip coplist_pal[] = {
    COP_MOVE(BPLCON0, BPLCON0_COMPOSITE_COLOR),
    COP_MOVE(COLOR00, 0x000),
    0x7c07, 0xfffe,            // wait for 1/3 (0x07, 0x7c)
    COP_MOVE(COLOR00, 0xf00),
    0xda07, 0xfffe,            // wait for 2/3 (0x07, 0xda)
    COP_MOVE(COLOR00, 0xff0),
    COP_WAIT_END
};

static UWORD __chip coplist_ntsc[] = {
    COP_MOVE(BPLCON0, BPLCON0_COMPOSITE_COLOR),
    COP_MOVE(COLOR00, 0x000),
    0x6607, 0xfffe,
    COP_MOVE(COLOR00, 0xf00),
    0xb607, 0xfffe,
    COP_MOVE(COLOR00, 0xff0),
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

int main(int argc, char **argv)
{
    SetTaskPri(FindTask(NULL), TASK_PRIORITY);
    BOOL is_pal = init_display();
    printf("PAL display: %d\n", is_pal);

    /*
        Settiamo il puntatore alla copperlist1, usiamo anche qui la variabile "custom" che
        il compilatore ci fornisce per accedere ai registri custom
    */
    custom.cop1lc = (ULONG) (is_pal ? coplist_pal : coplist_ntsc);
    waitmouse();  // replace with logic
    reset_display();
    return 0;
}