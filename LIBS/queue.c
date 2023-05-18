#include "queue.h"
#include "stddef.h"
#include "stdint.h"
/*
Here is a basic implementation of a  circular queue of length 256
We dont care if the queue is emnpty
*/
#define len 256

int begin =0;
int end =0;
int ar[len];

void reset_queue(){
    begin=0;
    end =0;
}

void enqueue(int n){
    if(end<len){
        ar[end++] =n; 
    }
    else{
        end = 0;
        ar[end++] =n;
    }
}

int dequeue(){
    if(begin==end){return -1;} //this diagnostic is no good but whatever
    if(begin<len){
        return ar[begin++];
    }
    else{
        int res = ar[begin];
        begin = 0;
        return res;
    }
    
}

int is_empty(){
    return begin==end;
}


