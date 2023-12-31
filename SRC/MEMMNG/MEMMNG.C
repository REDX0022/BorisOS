//here we have the memory manager, it is very simple, too simple
#include "stdint.h"
#include "stddef.h"
#include "kernel_out.h"


#define max_memory_sectors 50//change this later man okay? TODOOOOO
#define memory_begin 0x500 
#define memory_end 0x80000 
struct alloc_segment{
    uintptr_t begin;
    size_t len;
};

/// @brief this is a very simple strucuter, segments of len 0 are illegal, the last segment has len 0 and is the stop segment
struct alloc_segment memory[max_memory_sectors];


void start_program(){
    
    init_memory_manager();
    
}


void print_mem(){//this is debug func, shall not be in the lib
    for(int i = 0;i < max_memory_sectors;i++){
        printf((int)memory[i].begin);
        printch(' ');
        printf((int)(memory[i].len+memory[i].begin));
        nl();
    }

}


/// @brief initalizes the memory manager
/// @return success
int init_memory_manager(){
    //TODO: change this to the actual memory map which is used
    memory[0].begin = memory_begin; 
    memory[0].len = (size_t)(0x8000-memory_begin); //reserved by the os, before the memory manager was functional
    memory[1].begin = 0x10000; //STACK BEGIN
    memory[1].len = 0x10000; //ONE SEGMENT
    memory[2].begin = memory_end;
    memory[2].len = (size_t)0;
    return 0;
}

/// @brief Allocates memory, the user is responsible for deallocating it
/// @param size the size of memory to be allocated
/// @return pointer to the location of available memory
void* malloc(size_t size){
    for(int i = 0;memory[i].len && i < max_memory_sectors;i++){
        size_t cur_available = memory[i+1].begin-memory[i].begin-memory[i].len;
        if(cur_available>size){ //we have found the available space
            void* res =(void*) (memory[i].begin+memory[i].len);
            memory[i].len+=size;
            prints("MALLOC'd ",9);
            printf(size);
            printch('@');
            printf((int)res);
            nl();
            return res;
        }
        else if(cur_available==size && memory[i+1].len){ //we have found the available space but we need to merge the segments
            void* res =(void*) (memory[i].begin+memory[i].len);
            memory[i].len+=size;
            memory[i].len+=memory[i+1].len;
            for(int j = i+1;memory[j].len & j < max_memory_sectors;j++){//we shift the rest of the array one to the left
                memory[j] = memory[j+1];
                
            }
            prints("MALLOC'd ",9);
            printf(size);
            printch('@');
            printf((int)res);
            nl();
            return res;
        }
    }
    prints("MALLOC'd ",9);
    printf(size);
    printch('@');
    printf(0);
    nl();
    return NULL; //no memory slot found
}

//TODO this doesnt work for sure, we'll pretend it does, we'll never need it anyways
void dalloc(uintptr_t begin, size_t size){
    //we can have 3 cases here
    // - given segment is completly within another
    // - given segment messes with the part of another
    // - given segment spans multiple allocated segments
    prints("DALLOC'd ",9);
    printf((int)size);
    printch('@');
    printf((int)begin);
    nl();
    int removed_sectors_begin =-1;
    int removed_sectors_end = -1;
    for(int i = 0;memory[i].len && i < max_memory_sectors;i++){
        printch('l');
        if(memory[i].begin < begin && memory[i].begin + memory[i].len > begin + size){ //we need to split the sector in 2
            printch('a');
            struct alloc_segment temp;
            for(int j = max_memory_sectors-2; j > i+1; j--){
                memory[j+1] = memory[j];
            }

            temp = memory[i+1];

            memory[i+1].begin = begin+size;
            memory[i+1].len = memory[i].begin + memory[i].len - (begin + size);
            
            memory[i].len = begin-memory[i].begin;
         
            return; //we are done with the dalloc
        }
        if(memory[i].begin>=begin+size){
            printch('b');
            break;
        }
        if(memory[i].begin < begin+size && begin+size < memory[i].begin+memory[i].len){
            printch('c');
            memory[i].len = memory[i].begin+memory[i].len-(begin+size);
            memory[i].begin = begin+size;
            break;
        }
        if(memory[i].begin<begin&&memory[i].begin+memory[i].len>begin){
            printch('d');
            memory[i].len = begin-memory[i].begin;
        }

        if(memory[i].begin>=begin && removed_sectors_begin == -1){ //this segment is not nececary
            printch('e');
            
            removed_sectors_begin = i;
            
        }
        

        removed_sectors_end = i;
    }
    if(removed_sectors_begin==-1){return;} //this shouldnt happen, should throw an error here maybe?? or maybe it should

    int offset = removed_sectors_end-removed_sectors_begin+1;
    for(int i = removed_sectors_begin;memory[i].len & i+offset < max_memory_sectors;i++){
        memory[i] = memory[i+offset];
    }

}

/// @brief A rudementary memcpy
/// @param src 
/// @param dest 
/// @param n 
void memcpy(void *src,void *dest,size_t n){
    char *csrc = (char *)src;
    char *cdest = (char *)dest;

    for(int i =0;i<n;i++){
        cdest[i] = csrc[i];
    }

}



