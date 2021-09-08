#include <clib/exec_protos.h>
#include <stdio.h>

void point_bitplanes (UBYTE* bitplanes, UWORD* BPL1PTH_addr, int bpl_number);
UBYTE* init_bitplanes(size_t size);
void free_bitplanes(UBYTE* bitplanes);
void switchBuffers(UBYTE* bitplanes, UWORD* BPL1PTH_addr, int bpl_number,size_t bpls_size);
