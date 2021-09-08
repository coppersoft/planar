#include "disk.h"

void* alloc_and_load_asset(size_t size, char* filename) {
    UBYTE* dest = AllocVec(size,MEMF_CHIP|MEMF_CLEAR);

    load_asset(dest,size,filename);
    return dest;
}

void load_asset(UBYTE* dest, size_t size, char* filename) {
    FILE *fp = fopen(filename, "rb");
    if (fp) {
        fread(dest, sizeof(unsigned char), size, fp);
    } else {
        printf("error: file '%s' not found\n", filename);
    }
}