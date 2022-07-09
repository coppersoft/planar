#ifndef _DISK_
#define _DISK_

#include <clib/exec_protos.h>
#include <stdio.h>

void* alloc_and_load_asset(char* filename);
void load_asset(UBYTE* dest, char* filename);
size_t findSize(char* fileName);

#endif
