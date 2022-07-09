#include <hardware/intbits.h>
#include <exec/interrupts.h>
#include <clib/exec_protos.h>
#include <hardware/custom.h>

#include "interrupt.h"

/*
    http://amigadev.elowar.com/read/ADCD_2.1/Libraries_Manual_guide/node05EB.html


    e soprattutto https://wiki.amigaos.net/wiki/Exec_Interrupts
*/



// Struttura Interrupt, vedi
// http://amigadev.elowar.com/read/ADCD_2.1/Includes_and_Autodocs_2._guide/node008C.html
struct Interrupt vertb_interrupt;
    
/*
    L'interrupt che ci serve, il VERTB non è di tipo handler, ma di tipo server chains quindi non posso settarlo con
    SetIntVector (nell'esempio del cinese lui usava quelli AUDX che sono di tipo handler) ma con
    AddIntServer
*/
void installVertbInterrupt(APTR code, APTR data) {

    vertb_interrupt.is_Node.ln_Type = NT_INTERRUPT;
    vertb_interrupt.is_Node.ln_Pri = -60;
    vertb_interrupt.is_Data = data;
    vertb_interrupt.is_Code = code;

    AddIntServer(INTB_VERTB,&vertb_interrupt);
}

void uninstallVertbInterrupt() {
    RemIntServer(INTB_VERTB, &vertb_interrupt);
}
