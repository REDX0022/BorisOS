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
#include "MEMMNG.H"

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



 struct disk_packet{
    uint8_t size;
    uint8_t padding; 
    uint16_t num_of_sectors;
    uint16_t offset;
    uint16_t segment;
    uint32_t sector;
    uint32_t rest;
};

 struct directory{ //TODO: put unsigned where needed
    char name[11]; //the ext is also here 
    uint8_t attribute;
    uint8_t NT_reserved;
    uint8_t creation_stamp;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access_date;
    uint16_t reserved_for_fat32;
    uint16_t last_write_time;
    uint16_t last_write_date;
    uint16_t starting_cluster;
    uint32_t file_size_in_bytes; //
};

char temp_sector[bytes_per_sector]; //these 2 are used by functions only while the function is executing
char temp_sector2[bytes_per_sector]; 

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
    
    uint16_t *FAT_search_sector = &temp_sector;
    
    uint16_t next_cluster = dir.starting_cluster;
    
    //follow the FAT table
    while(1){
        
        load_sector(data_start+(next_cluster-2),(void*) pos); // load the file sector

        load_sector(FAT_start+(next_cluster*2)/bytes_per_sector,(void*)FAT_search_sector); //load the fat search sector

        next_cluster = FAT_search_sector[next_cluster]; //look for the next cluster
        if(next_cluster>=0xFFF8){break;} //TODO: add support for bad sectors

        pos +=bytes_per_sector;
    }
    memcpy((void*)temp_sector2,(void*)save_start,save_size); //retrives the rest
    return 0;
}

/// @brief returnds a null terminated array of files and folders in the given directory, memory should be deallocated later
struct directory* list_dir(struct directory folder){ //we already know the length of this list because its in the dir desc
    if(!is_volume&&!is_folder){return NULL;}

    //first we want to find out the size of the dir
    uint32_t size_in_sectors = folder.file_size_in_bytes;

    struct directory* result = (struct directory*) malloc(folder.file_size_in_bytes); //this should be deallocated when done

    //we want to save the last part of the last sector
    
    
    
    load_file(folder,(char*)result);

   
    return result;
}

struct directory* search_dir(struct directory folder, char name[12]){
    if(!is_volume&&!is_folder){return NULL;}
    //first we load the folder into memory
    struct directory* search_place = list_dir(folder);
    for(struct directory* i = search_place;i<search_place+folder.file_size_in_bytes;i++){

    }

}


void pad_name(char** name){
    
    
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

