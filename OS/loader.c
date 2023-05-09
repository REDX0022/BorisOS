//we have gotten into C
//the early loader
//it should load the memory manager
// the loader should have a basic loader to load from path
#include "stdint.h"
#include "stdbool.h"

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
//assign where to put the sectors
#define search_sector_address 0x7E00
#define disk_packet_struct 0x8000
//--------------------------------------------------------
//we assign where the loader will be
#define loader_memory_address 0x500


typedef struct disk_packet{
    int8_t size;
    int8_t padding; 
    int16_t num_of_sectors;
    int16_t offset;
    int16_t segment;
    int32_t sector;
    int32_t rest;
};

typedef struct directory{
    char name[11]; //the ext is also here 
    int8_t attribute;
    int8_t NT_reserved;
    int8_t creation_stamp;
    int16_t creation_time;
    int16_t createion_date;
    int16_t last_access_date;
    int16_t reserved_for_fat32;
    int16_t last_write_time;
    int16_t last_write_date;
    int16_t starting_cluster;
    int16_t file_size_in_bytes; //
};

typedef struct MZ{
    
};


struct directory search_sector[16]; //this needs to change with bytes_per_sector

int16_t FAT_search_sector[256]; //bytes_per_sector

int load_sector(int sector_pos,int memory_pos){

   struct disk_packet dp;
   dp.size = 0x10;
   dp.padding = 0;
   dp.num_of_sectors = 1;
   dp.offset = memory_pos%16;
   dp.segment = memory_pos/16;
   dp.sector = sector_pos;
   dp.rest =0;
   load_sector_helper(&dp);
    return 0; //TODO: Make actuall diagonsitcs

}





//we are doing this so we can access the disk packet pos via the stack
void load_sector_helper(struct disk_packet *ptr){
    asm(
    //mov byte [disk_packet_struct], 0x10 ;size of packet is 16 bytes
    //mov byte [disk_packet_struct+1],0 ; always 0
    //mov word [disk_packet_struct+2],1 ; number of sectors to transfer
    //mov word [disk_packet_struct+4], (search_sector_address) ;offset of placement
    //mov word [disk_packet_struct+6], 0 ;segment of placement
    //mov dword [disk_packet_struct+8] , edx ; this is in sectors
    //mov dword [disk_packet_struct+12],0 ; should a word or a dword be here?? i have no clue, because its 32 bit i think its word but whatever
    //--------------------------call int 13h-----------------------
        "pushad ;"
        "push ds ;"
        "mov dl, 0x80 ; TODO make this flexible;"
        "xor ax, ax ;"
        "mov ds, ax ;"
        "mov ah, 0x42 ;"
        "mov esi, [bp+38] ; +4 for esp, +32 for pushad, +2 for ds"
        "ror esi, 4 ; "
        "mov ds, si ; "
        "shr esi, 28 ;"
        "int 0x13 ;"
        "pop ds ;"
        "popad;"
    //----------------------------------------------------------
    );

}


bool cmp_name(char str1[11], char *str2){
    //the first is len 11 the second is terminated by / or null
    bool string_ended =0;
    int j =0;
    for(int i =0;i<8;i++){
        if(str2[j]=='.'){
            //if we need to compare extensins
            j++;
        }
        if(str2[j] == '/'|| str2[j] ==0){
            return 1;
        }
        if(str1[i] != str2[j]){
            return 0;
        }
        j++;
    }   
    return 1;
}

/// @brief loads a file into memory, overwrites the sector remainder, so its bytes_per_sector alligned
/// @param path null terminated string with path to the file
/// @param pos the position in memory
/// @return 0 if sucessful, otherwise not
int load_file(char* file_name,int pos){
    //we need to search for the directory
    //this early loader loads only from root dir
    int16_t next_cluster = -1;
    for(int i =0;i<root_dir_size;i++){

        load_sector(root_dir+i,&search_sector);
        for(int j =0;j<16;j++){ //bytes_per_sector

            if(cmp_name(search_sector[j].name,file_name)){
                next_cluster = search_sector[j].starting_cluster;
            
            } 
        }
    }

    if(next_cluster = -1){
        return 1;
    }
    
    
    
    while(true){
        load_sector(data_start+(next_cluster-2),pos); // load the file sector

        load_sector((next_cluster*2)/bytes_per_sector,&FAT_search_sector); //load the fat search sector

        next_cluster = FAT_search_sector[(next_cluster*2)%bytes_per_sector]; //look for the next cluster
       
        if(next_cluster>=0xFFF8){break;} //TODO: add support for bad sectors

        pos +=bytes_per_sector;
    }

}




int __start__(){



}

