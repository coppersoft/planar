#include "disk.h"

void* alloc_and_load_asset(size_t size, char* filename) {
    UBYTE* dest = AllocMem(size,MEMF_CHIP|MEMF_CLEAR);

    FILE *fp = fopen(filename, "rb");
    if (fp) {
        fread(dest, sizeof(unsigned char), size, fp);
    } else {
        printf("error: file '%s' not found\n", filename);
    }
    return dest;
}