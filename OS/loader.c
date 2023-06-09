//we have gotten into C
//the early loader
//it should load the memory manager
// the loader should have a basic loader to load from path
#include "stdint.h"
#include "stddef.h"

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
//we assign where the loader will be, why dont i let it be he can manage his own memory man
#define loader_memory_address 0x500
#define max_loaded_shared_libs 64
#define max_loaded_shared_func 256
//-----------------------------------------------------------
#define memmory_manager_address 0x5000

//============================Structures=======================================
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



struct MZext_header{
    char MZ[2]; //this should be MZ always
    uint16_t last_sector_bytes;
    uint16_t sector_num;
    uint16_t no_of_rel; //not used
    uint16_t header_size; //in 16-byte paragraphs, should be edited to accomodate library names
    uint16_t min_alloc;
    uint16_t max_alloc;
    int16_t ss_reg;
    uint16_t sp_reg;
    uint16_t checksum; //usually 0
    uint16_t ip_reg;
    uint16_t cs_reg;
    uint16_t reloc_offset; // not used
    uint16_t overlay_num; //usually unused
    char *lib_name[16]; //only the first 11 characers are used
    
};


struct shared_lib_map{
    uint16_t size;
    uint16_t offsets;
};

struct shared_lib{
    char name[11];
    uint16_t *funs_ptr;
    uint16_t count;
};

//==============================================================================================


//=============================Global variables=========================================
char temp_sector[bytes_per_sector];
char temp_sector_2[bytes_per_sector];


struct shared_lib shared_libs[max_loaded_shared_libs];
uint16_t shared_func[max_loaded_shared_func];

struct shared_lib *shared_libs_ptr = &shared_libs[0]; //the pointer to the next available space//idk if the compiler supports this !!!!
uint16_t *shared_func_ptr = &shared_func[0]; //pointer to the available space



//=====================================================================================


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

/// @brief loads a file into memory, OVERWRITES the sector remainder, so its bytes_per_sector alligned
/// @param path null terminated string with path to the file
/// @param pos the position in memory
/// @return 0 if sucessful, otherwise not
int load_file(char* file_name,char *pos){
    struct directory *search_sector = &temp_sector;
            
    //we need to search for the directory
    //this early loader loads only from root dir
    uint16_t next_cluster = 0; //cluster 0 isn't allowed by spec
    for(int i =0;i<root_dir_size;i++){

        load_sector(root_dir+i,(void*)search_sector); 
        for(int j =0;j<16;j++){ //bytes_per_sector

            

            if(cmp_name(search_sector[j].name,file_name)){
                next_cluster = search_sector[j].starting_cluster;
                
            } 
        }
    }
   
    if(!next_cluster){
        return 1;
    }
    

    uint16_t *FAT_search_sector = &temp_sector;
    
    //follow the FAT table
    while(1){
        
        load_sector(data_start+(next_cluster-2),(void*) pos); // load the file sector

        load_sector(FAT_start+(next_cluster*2)/bytes_per_sector,(void*)FAT_search_sector); //load the fat search sector

        next_cluster = FAT_search_sector[next_cluster]; //look for the next cluster
        if(next_cluster>=0xFFF8){break;} //TODO: add support for bad sectors

        pos +=bytes_per_sector;
    }
    return 0;
}







//TODO: Pass something to the programm at start
typedef void (* init) (); 

/// @brief Starts a kernel space programm
/// @param start memory loaction of the programm, should be a void pointer, cant be because of arithmetic
/// @return success
int start_kernel_programm(void *start){
    if(!start){return -1;} //null pointer exeption
    struct MZext_header* mz;
    mz = ((struct MZext_header*) start); //there might need to be changes if there i
       
    //here we dont read the libraries needed because that is predefned??
    
    init execution_start; 
    execution_start = (init) ((char*)mz + mz->header_size*16+mz->cs_reg*16+mz->ip_reg);
    //but we do need to find the libraries that it provides
    
    execution_start(); //for now we use a global stack for the entire os 
    return 0;
}

int load_map(char* name){ 
   
    if(load_file(name,(void *)&temp_sector_2)){
        return 1;
    }

    struct shared_lib_map* map =  (struct shared_lib_map*)(&temp_sector_2); //znas da si nesto sjebo kad double kastujes pointere

    memcpy(name,(void*) &(shared_libs_ptr->name),11);
    shared_libs_ptr->count = map->size;
    shared_libs_ptr->funs_ptr = shared_func_ptr;

    memcpy((void*)(&map->offsets),shared_func_ptr,(2*map->size)); // this is part of the map spec

    shared_func_ptr += map->size;
    shared_libs_ptr++;
    return 0;
}




int __start__(){
    //we load the operating system slowly
    


   
 
    //we wanna load the memory manager
    
    char name[11] = "MEMMNG  SYS";
    char map_name[11] = "MEMMNG  MAP";
    if(load_file(name,memmory_manager_address)){
        printf(14); //failed to load file
    }
    if(start_kernel_programm(memmory_manager_address)){
        printf(15); //failed to start mem mng
    }
    if(load_map(map_name)){
        printf(16);
    }
   
    
}

void printf(int n){
    if(n==0){printch('0');return;}
    printf(n/10);
    char tmpprint =((uint32_t)n%10)+48;
    printch(tmpprint&0xFF); //48 is the offset of the char 0
    n/=10;
    
}

void printch(char c){ //i dont know it its needed to push, its for safety
    asm("pushad \n"
        "mov bh, 0x00 \n"
        "mov al, [bp+8] ; +4 for ebx,  \n"
        "mov ah, 0x0e \n"
        "int 0x10 \n"
        "popad \n"
    );
}


void prints(char* ptr, size_t len){
    for(int i =0;i<len;i++){
        printch(*(ptr++));
    }
}

char hex[16] = "0123456789ABCDEF";
void dmph(char* ptr, size_t len){
    for(int i =0;i<len;i++){
        char high = (char)(hex[((*ptr)&0xFF)>>4]);
        char low = (char)(hex[(*ptr)&0xF]);
    
        printch(high);
        printch(low);
        printch(' ');
        ptr++;
    }
    
}


