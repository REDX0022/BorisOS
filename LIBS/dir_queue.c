#include "queue.h"
#include "stddef.h"
#include "stdint.h"
#include "FAT16_structs.h"
/*
Here is a basic implementation of a  circular queue of length 256
This file format is in directory
*/
#define len 256

int begin =0;
int end =0;
struct directory ar[len];

void reset_queue(){
    begin=0;
    end =0;
}

void dir_enqueue(struct directory dir){
    if(end<len){
        ar[end++] =dir; 
    }
    else{
        end = 0;
        ar[end++] =dir;
    }
}

void dir_dequeue(struct directory* ret){
    if(begin<len){
        *ret = ar[begin++];
    }
    else{
        begin = 0;
        *ret = ar[begin];
    }
    
}

int dir_is_queue_empty(){
    return begin==end;
}


