#include "disk.h"

size_t findSize(char* fileName) {
    FILE* fp = fopen(fileName, "r");

    if (fp == NULL) {
        printf("File %s not found\n",fileName);
        return -1;
    }

    fseek(fp,0,SEEK_END);
    size_t size = ftell(fp);
    fclose(fp);

    return size;
} 

static void load_asset_with_size(UBYTE* dest, size_t size, char* filename) {

    FILE *fp = fopen(filename, "rb");
    if (fp) {
        fread(dest, sizeof(unsigned char), size, fp);
    } else {
        printf("error: file '%s' not found\n", filename);
    }
    fclose(fp);
}

void* alloc_and_load_asset(char* filename) {
    size_t size = findSize(filename);

    UBYTE* dest = AllocVec(size,MEMF_CHIP|MEMF_CLEAR);

    load_asset_with_size(dest,size,filename);
    return dest;
}

void load_asset(UBYTE* dest, char* filename) {
    size_t size = findSize(filename);
    load_asset_with_size(dest,size,filename);
}

