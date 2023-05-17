#include "stddef.h"
#include "stdint.h"





void* malloc(size_t size);

void dalloc(uint32_t begin, size_t size);

void memcpy(void* src, void *dest, size_t n);

void init_memmng(u funcs){
    
}
