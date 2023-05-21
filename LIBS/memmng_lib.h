#include "stddef.h"
#include "stdint.h"

#define func_num 3

typedef void* (*malloc_ptr)(size_t size);
malloc_ptr malloc;

typedef void (*dalloc_ptr)(uint32_t begin, size_t size);
dalloc_ptr dalloc;

typedef void (*memcpy_ptr)(void* src, void *dest, size_t n);
memcpy_ptr memcpy;


typedef void (*print_mem_ptr)(void* src, void *dest, size_t n);
memcpy_ptr print_mem;



void init_MEMMNG(void** funcs){ //check if this is safe
    malloc = (malloc_ptr)(funcs[0]);
    dalloc = (dalloc_ptr)(funcs[1]);
    memcpy = (memcpy_ptr)(funcs[2]);
    print_mem = (print_mem_ptr)(funcs[3]);
}
