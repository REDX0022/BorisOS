//we have gotten into C
//the early loader
//it should load the memory manager
// the loader should have a basic loader to load from path
#include "stdint.h"
#include "stddef.h"
#include "kernel_out.h"
#include "FAT16_structs.h"
#include "memmng_lib.h"

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
#define disk_address_packet_struct 0x8000
//--------------------------------------------------------
//we assign where the loader will be, why dont i let it be he can manage his own memory man
#define loader_memory_address 0x500
#define max_loaded_shared_libs 64
#define max_loaded_shared_func 256
//-----------------------------------------------------------
#define memmory_manager_address 0x5000
#define file_manager_size 0x1500 //TODOO: change this perhaps, its just a perdiction on how much it will grow

//============================Structures=======================================
 



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
    char padding[4];    
    char (*lib_name)[16]; //only the first 11 characers are used
    
};


struct shared_lib_map{
    uint16_t size;
    uint16_t offsets;
};

struct shared_lib{
    char name[11];
    void** funs_ptr;
    uint16_t count;
};

//==============================================================================================


//=============================Global variables=========================================
char temp_sector[bytes_per_sector];
char temp_sector_2[bytes_per_sector];


struct shared_lib shared_libs[max_loaded_shared_libs];
void* shared_func[max_loaded_shared_func];

struct shared_lib *shared_libs_ptr = shared_libs; //the pointer to the next available space//idk if the compiler supports this !!!!
void** shared_func_ptr = shared_func; //pointer to the available space



//=====================================================================================


/// @brief A rudementary memcpy_loader
/// @param src 
/// @param dest 
/// @param n 
void memcpy_loader(void *src,void *dest,size_t n){
    char *csrc = (char *)src;
    char *cdest = (char *)dest;

    for(int i =0;i<n;i++){
        cdest[i] = csrc[i];
    }

}


struct disk_address_packet dap;

int load_sector(int sector_pos, void *memory_pos){ //idk if char pointer is good here

   dap.size = 0x10;
   dap.padding = 0;
   dap.num_of_sectors = 1;
   dap.offset = (uint16_t)(((uint32_t )memory_pos )%16);
   dap.segment = (uint16_t)(((uint32_t)memory_pos)/16);
   dap.sector = sector_pos;
   dap.rest =0;
 //TODO: Make actuall diagonsitcs
   load_sector_helper(&dap);
   return 0;

}





//we are doing this so we can access the disk packet pos via the stack
int load_sector_helper(struct disk_address_packet *ptr){
    asm(
    //mov byte [disk_address_packet_struct], 0x10 ;size of packet is 16 bytes
    //mov byte [disk_address_packet_struct+1],0 ; always 0
    //mov word [disk_address_packet_struct+2],1 ; number of sectors to transfer
    //mov word [disk_address_packet_struct+4], (search_sector_address) ;offset of placement
    //mov word [disk_address_packet_struct+6], 0 ;segment of placement
    //mov dword [disk_address_packet_struct+8] , edx ; this is in sectors
    //mov dword [disk_address_packet_struct+12],0 ; should a word or a dword be here?? i have no clue, because its 32 bit i think its word but whatever
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

        next_cluster = FAT_search_sector[next_cluster%(bytes_per_sector/2)]; //look for the next cluster
        if(next_cluster>=0xFFF8){break;} //TODO: add support for bad sectors

        pos +=bytes_per_sector;
    }
    return 0;
}

/// @brief 
/// @param name 
/// @return Pointer to a saved shared lib  
struct shared_lib* get_shared_lib(char* name){
    for(struct shared_lib* ptr = shared_libs;ptr!=shared_libs_ptr;ptr++){
        if(cmp_name(&(ptr->name),name)){return ptr;}
    }
    return NULL;

}




//TODO: argv and argc
typedef void (* init) (void** libs); 

/// @brief Starts a kernel space programm
/// @param start memory loaction of the programm, should be a void pointer, cant be because of arithmetic
/// @return success
int start_kernel_programm(void *start){
    if(!start){return -1;} //null pointer exeption
    struct MZext_header* mz;
    mz = ((struct MZext_header*) start); //there might need to be changes if there i
    
    dmph((char*)start,60);
    nl();
    void*** lib_store = &temp_sector; //where in the temp sector to put the funcs
    for(char (*lib_search)[16] = mz->lib_name; (*lib_search)[0]; lib_search++){
        prints(mz->lib_name,11);
        struct shared_lib* sh = get_shared_lib(lib_search);
        if(sh==NULL){
            return 2; // LIB NOT FOUND
        }
        nl();
        printf((int)sh);
        nl();
        *lib_store = sh->funs_ptr;
        lib_store++;
        
    }
    



    init execution_start; 
    execution_start = (init) ((char*)mz + mz->header_size*16+mz->cs_reg*16+mz->ip_reg);
    //but we do need to find the libraries that it provides
    nl();
    dmph(&temp_sector,10);
    nl();
    execution_start(&temp_sector); //for now we use a global stack for the entire os 
    return 0;
}

int load_kernel_map(char* name, char* pos){ 
   
    if(load_file(name,(void *)&temp_sector_2)){
        return 1;
    }

    struct shared_lib_map* map =  (struct shared_lib_map*)(&temp_sector_2); //znas da si nesto sjebo kad double kastujes pointere

    memcpy_loader(name,(void*) &(shared_libs_ptr->name),11);
    shared_libs_ptr->count = map->size;
    shared_libs_ptr->funs_ptr = shared_func_ptr;

    for(uint16_t i =0;i<map->size;i++){//we store the pointers and add the program begin
        *shared_func_ptr = (void*) (pos + *(&(map->offsets)+i)); //this hack is so stupid i dont want to look at it anymore
        shared_func_ptr++;
    }

    shared_func_ptr += map->size;
    shared_libs_ptr++;
    return 0;
}


//

int load_map(char* name, char* pos){ //TODO: SHOULD USE THE MALLOC AND MEMORY MANAGER TIME 
   
    if(load_file(name,(void *)&temp_sector_2)){
        return 1;
    }

    struct shared_lib_map* map =  (struct shared_lib_map*)(&temp_sector_2); //znas da si nesto sjebo kad double kastujes pointere

    memcpy_loader(name,(void*) &(shared_libs_ptr->name),11);
    shared_libs_ptr->count = map->size;
    shared_libs_ptr->funs_ptr = shared_func_ptr;

    for(uint16_t i =0;i<map->size;i++){//we store the pointers and add the program begin
        //*shared_func_ptr = (void*) (pos + (map->offsets+ sizeof(uint16_t)*i)); //i hope to god this works
        shared_func_ptr++;
    }

    shared_func_ptr += map->size;
    shared_libs_ptr++;
    return 0;
}


int __start__(){
    //we load the operating system slowly
    


   
 
    //LOADING THE MEMORY MANAGER
    
    char name[11] = "MEMMNG  SYS";
    char map_name[11] = "MEMMNG  MAP";
    if(load_file(name,memmory_manager_address)){
        prints("FAILED TO LOAD MEMORY MANAGER",30);
        nl();
    }
    else{
        prints("LOADED MEMORY MANAGER SUCCESFULLY",34);
        nl();
    }
    if(start_kernel_programm(memmory_manager_address)){
        prints("FAILED TO START MEMORY MANAGER",31);
        nl();
    }
    else{
        prints("STARTED MEMORY MANAGER SUCCESFULLY",35);
        nl();
    }
    if(load_kernel_map(map_name,memmory_manager_address)){
        prints("FAILED TO LOAD MEMORY MANAGER MAP",34);
        nl();;  
    }
    else{
        struct shared_lib* sh = get_shared_lib(&map_name);
        init_MEMMNG(sh->funs_ptr);
        prints("LOADED MEMORY MANAGER MAP",26);
        nl();
    }
    
    //LOADING THE FILE SYSTEM MANAGER
    
    char name1[11] = "FILEMNG SYS";
    char map_name1[11] = "FILEMNG MAP";

    void* file_manager_address = malloc(file_manager_size);
    printf(file_manager_address);
    nl();
    if(load_file(name1,file_manager_address)){
        prints("FAILED TO LOAD FILE MANAGER",28);
        nl();
    }
    else{
        prints("LOADED FILE MANAGER SUCCESFULLY",32);
        nl();
    }
    if(start_kernel_programm(file_manager_address)){
        prints("FAILED TO START FILE MANAGER",29);
        nl();
    }
    else{
        prints("STARTED FILE MANAGER SUCCESFULLY",33);
        nl();
    }
    if(load_kernel_map(map_name1,file_manager_address)){
        prints("FAILED TO LOAD FILE MANAGER MAP",32);
        nl();;  
    }
    else{
        struct shared_lib* sh = get_shared_lib(name);
        init_MEMMNG(sh->funs_ptr);
        prints("LOADED FILE MANAGER MAP",24);
        nl();
    }


}

