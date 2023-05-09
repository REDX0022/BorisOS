//here we have the memory manager
//it wont be alligned but it will have an option to allign it to the 

#define max_memory_sectors 1000

memory_sector memory[max_memory_sectors];


typedef struct memory_sector
{
    bool available;
    int begin;
    int length;
    int prev_index;
    int next_index;

};




//we need to make a primitive version of a linked list without using linked list because we dont have malloc



