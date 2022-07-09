#include <clib/exec_protos.h>

// Occhio, APTR è semplicemente un typedef void* , ovvero un puntatore a 32 bit senza tipo
void installVertbInterrupt(APTR code,APTR data);
void uninstallVertbInterrupt();
