/*
This should form the basic premise of a file system manager
fat 16
functions:

directory* dir(struct directory cur) {} //this should be null terminated man

directory* search(char* path, char* name) {}

int from_path(struct *directory dir, char* path) {}

int load_file(struct directory file) {} //we need to check if its the file

int modify_file(struct *directory file) {} 

int write_file(char* path, char name, (void*) pos, size_t size) {}



*/
#include "stddef.h"
#include "stdint.h"
#include "memmng_header.h"
#include "dir_queue.h"
#include "FAT16_structs.h"

//========================Variables for the FAT16 file system==========================
#define jump_point 0x3E
#define OEM_ID 'BSD  4.4' 
#define bytes_per_sector 512
//in code we assume this is always 1
#define sectors_per_clutser 1 
#define reserved_sectors 1
#define num_of_FATs 1
//each directory is 32
//please make it divisible by 16
#define num_of_root_dir 512 
#define small_sector_num 0xFFFF
#define media_descriptor 0xF8
//this should be calcualed according to small_sector_num
#define sectors_per_FAT 61
#define sectors_per_track 0
#define num_of_heads 1
//start of volume basically
#define hidden_sectors 0
#define large_sector_num 0
#define drive_num 0x80
#define reserved 0
#define extended_boot_sig 0x29
//idk what this is MacOS put it there
#define volume_serial_num 0x93811B4
//should be the same as the root dir
#define volume_label 'BORISOSVOL '
#define file_system_type 'FAT16   '
//-----------------------------------
//Calculated starting points, in sectors
#define volume_start hidden_sectors
#define FAT_start volume_start + reserved_sectors 
#define root_dir FAT_start+num_of_FATs*sectors_per_FAT
#define root_dir_size (num_of_root_dir*32)/bytes_per_sector
#define data_start root_dir+root_dir_size
//--------------------------------------------------------


struct directory volume;
int cached_FAT_sector =0;
uint16_t FAT_cache[bytes_per_sector/2]; //this is used to cache one sector of fat

char temp_sector[bytes_per_sector]; //these 2 are used by functions only while the function is executing
char temp_sector2[bytes_per_sector]; 




/// @brief rudementary file name comparison, root dir only
/// @param str1 the name in the directory
/// @param str2 the name in the path(string)
/// @return 
int cmp_name(char* str1, char* str2){
    //this just compares the root dir strings
    for(int i =0;i<11;i++){
        if(str1[i]!=str2[i]){
           
            return 0;
        }
    }
    return 1;
}

uint16_t FAT_lookup(uint16_t cluster){
    int needed_sector = FAT_start+(cluster*2)/bytes_per_sector;
    if(needed_sector = cached_FAT_sector){ //the sector we need is cached
        return FAT_cache[(cluster%(bytes_per_sector/2))];//TODO: check this calc
    }
    else{ //we need to load a new sector
        load_sector(needed_sector,(void*) &FAT_cache);
        return FAT_cache[(cluster%(bytes_per_sector/2))];//TODO: check this calc
    }
}

uint16_t FAT_edit(u_int16_t clutser){
    
}

/// @brief Gets the directory from a null terminated path string
/// @param path doesn't need to be in standard form 
/// @return directory somewhere in memory
struct directory* from_path(char* path){//this string exists on the stack, it is null terminated
    struct directory cur = volume;
    char* ptr=path;
    while(*ptr!='\0'){
        int is_cur_folder = 1; //or something stupid
        size_t cur_len =0; //this is the length of one file or folder name
        char* dir_name = ptr;
        while(*ptr!='/'&&*ptr!='\0'){
            cur_len++;
            if(*ptr =='.'){is_cur_folder=0;}
        }
        char padded_name[12] = "\0\0\0\0\0\0\0\0\0\0\0"; //this is all 0s
        memcpy(dir_name,padded_name,cur_len);
        if(!is_cur_folder){ //we don't need to pad the folder name
            pad_file_name((char**) &padded_name);
        }
        struct directory* ret = search_dir(cur,padded_name); //we need to dealoate the return 
        if(ret==NULL){return NULL;}
        cur = *ret;
        dalloc((uint32_t)ret,sizeof(directory));
    }
}



struct disk_packet dp;

int load_sector(int sector_pos, void *memory_pos){ //idk if char pointer is good here

   dp.size = 0x10;
   dp.padding = 0;
   dp.num_of_sectors = 1;
   dp.offset = (uint16_t)(((uint32_t )memory_pos )%16);
   dp.segment = (uint16_t)(((uint32_t)memory_pos)/16);
   dp.sector = sector_pos;
   dp.rest =0;
 //TODO: Make actuall diagonsitcs
   load_sector_helper(&dp);
   return 0;

}





//we are doing this so we can access the disk packet pos via the stack
int load_sector_helper(struct disk_packet *ptr){
    asm(
    //mov byte [disk_packet_struct], 0x10 ;size of packet is 16 bytes
    //mov byte [disk_packet_struct+1],0 ; always 0
    //mov word [disk_packet_struct+2],1 ; number of sectors to transfer
    //mov word [disk_packet_struct+4], (search_sector_address) ;offset of placement
    //mov word [disk_packet_struct+6], 0 ;segment of placement
    //mov dword [disk_packet_struct+8] , edx ; this is in sectors
    //mov dword [disk_packet_struct+12],0 ; should a word or a dword be here?? i have no clue, because its 32 bit i think its word but whatever
    //--------------------------call int 13h-----------------------
        "pushad \n"
        "push ds \n"
        "mov dl, 0x80  ; TODO make this flexible; \n"
        "xor ax, ax  \n "
        "mov ds, ax  \n "
        "mov ah, 0x42 \n "
        "mov esi, dword [bp+8] ; +4 esp +4 size \n" 
        "ror esi, 4 \n "
        "mov ds, si \n "
        "shr esi, 28 \n "
        "int 0x13 \n "
        "jnc skip \n"
        "mov bh, 0 \n"
        "mov al, 'f' \n"
        "mov ah, 0x0e \n"
        "int 0x10 \n"
        "skip: \n"
        "pop ds \n "
        "popad \n "
    // ----------------------------------------------------------
    );
   

}




/// @brief loads a file into memory, saves the remainder sector unused 
/// @param path null terminated string with path to the file
/// @param pos the position in memory
/// @return 0 if sucessful, otherwise not
int load_file(struct directory dir,char *pos){
    
    size_t save_size = bytes_per_sector - ((uintptr_t) pos + dir.file_size_in_bytes) % bytes_per_sector; //this could be +-1 idk TODO

    char *save_start = (char*) pos + dir.file_size_in_bytes;

    memcpy((void*)save_start,(void*)temp_sector2,save_size); //saves the rest
    
   
    
    uint16_t next_cluster = dir.starting_cluster;
    
    //follow the FAT table
    while(1){
        
        load_sector(data_start+(next_cluster-2),(void*) pos); // load the file sector

        

        next_cluster = FAT_lookup(next_cluster); //look for the next cluster
        
        if(next_cluster>=0xFFF8){break;} //TODO: add support for bad sectors

        pos +=bytes_per_sector;
    }
    memcpy((void*)temp_sector2,(void*)save_start,save_size); //retrives the rest
    return 0;
}

/// @brief returnds a null terminated array of files and folders in the given directory, memory should be deallocated later
struct directory* list_dir(struct directory folder){ //we already know the length of this list because its in the dir desc

   if(!is_folder(folder)){return NULL;}

    //first we want to find out the size of the dir
    uint32_t size_in_sectors = folder.file_size_in_bytes;

    struct directory* result = (struct directory*) malloc(folder.file_size_in_bytes); //this should be deallocated when done

    //we want to save the last part of the last sector
    
    
    
    load_file(folder,(char*)result);

   
    return result;
}
struct directory* list_root(){
    struct directory* result = (struct directory*) malloc(root_dir_size*bytes_per_sector); //this should be deallocated when done
    int cur_sector = root_dir;
    for(char* ptr = (char*)(root_dir*bytes_per_sector);ptr!=(char*)((root_dir+root_dir_size)*bytes_per_sector);ptr+=bytes_per_sector){
        load_sector(cur_sector,(void*)result);
        cur_sector++;
    }
    return result;
}


/// @brief BFS
/// @param folder the folder in which we want to search, can also be a volum 
/// @param name name
/// @return the first file it finds
struct directory* search(struct directory folder,char name[11]){
    if(!is_volume(folder)&&!is_folder(folder)){return NULL;}
    reset_queue();
    dir_enqueue(folder);
    while(!dir_is_queue_empty()){ //we try to empty the queue
        struct directory cur;
        dir_dequeue(&cur);
        struct directory* res = search_dir(cur, name);
        if(res!=NULL){return res;} //we have found the file somewhere in there
    }
    return NULL;
}



/// @brief Does a BFS on the entire directory, places a directory in memory and returns a pointer to it, returns the first file it finds, reset_queue needs to be called before
/// @param folder folder in which we want to search, can also be volume
/// @param name standardized name
struct directory* search_dir(struct directory folder, char name[11]){
    //if(!is_volume(folder)&&!is_folder(folder)){return NULL;}
    
    //first we load the folder into memory
    struct directory* search_place;
    size_t search_size;
    if(is_volume(folder)){//we search the root dir
        search_place = list_root();
        search_size =  root_dir_size*bytes_per_sector;
    }
    else{
        search_place= list_dir(folder);
        search_size= folder.file_size_in_bytes;
    } 
    for(struct directory* i = search_place;i<search_place+folder.file_size_in_bytes;i++){
        
        if(is_folder(*i)){//we queue up a folder to be searched later recursively
            dir_enqueue(*i);//if the queue overfills this doesn't work
        }
        
        if(cmp_name(name,i->name)){ //we have found the file yayy
            void* res = malloc(sizeof(directory));
            memcpy(i,res,sizeof(directory));
             dalloc((uint32_t)search_place,folder.file_size_in_bytes); //we need to dealocate the memory, we don't want no leaks
            return (struct directory*) res;
        }

    }
    dalloc((uint32_t)search_place,folder.file_size_in_bytes); //we need to dealocate the memory, we don't want no leaks
    return NULL;

}

void init(){
    //we want to setup the root dir
    char temp_volume_name[11] = "BORISOSVOL"; //this changes with volume_label
    memcpy(&temp_volume_name,&volume.name,11);
    volume.attribute = 0b0001;

}

/// @brief adds standardized padding
/// @param name pointer to the name of the string being edited
/// @return returns 1 if the opeartion can't be completed, otherwise 0
int pad_file_name(char* name[12]){//TODO, idk man im too lazy it should convert names to a standard format
    int dot_pos =-1;
    for(int i =0;i<11;i++){
        if((*name)[i] =='.'){
            dot_pos = i;
        }
    }
    if(dot_pos==-1 || dot_pos>8){return 1;} //file name isn't valid
    (*name)[dot_pos] = 0;
    //now we have to move the extension
    char temp_ext[3];
    temp_ext[0] =  (*name)[dot_pos+1];
    temp_ext[1] =  (*name)[dot_pos+2];
    temp_ext[2] =  (*name)[dot_pos+3];
    (*name)[10] = temp_ext[2];
    (*name)[9] = temp_ext[1];
    (*name)[8] = temp_ext[0];
}


//checks what type of directory it is
int is_read_only(struct directory dir){
    return dir.attribute & 0b1;
}

int is_hidden(struct directory dir){
    return dir.attribute & 0b01;
}

int is_system(struct directory dir){
    return dir.attribute & 0b001;
}

int is_volume(struct directory dir){
    return dir.attribute & 0b0001;
}

int is_folder(struct directory dir){
    return dir.attribute & 0b00001;
}

int is_archive(struct directory dir){
    return dir.attribute & 0b000001;
}

