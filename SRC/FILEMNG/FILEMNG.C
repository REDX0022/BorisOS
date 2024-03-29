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
#include "kernel_out.h"
#include "memmng_lib.h"
#include "dir_queue.c" //this has fat16 structs


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

struct directory directory_size;
struct directory volume;
int cached_FAT_sector = -1;
uint16_t FAT_cache[bytes_per_sector/2]; //this is used to cache one sector of fat

char temp_sector[bytes_per_sector]; //these 2 are used by functions only while the function is executing
char temp_sector2[bytes_per_sector]; 

char padding[256];
struct disk_address_packet dap;

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
    if(needed_sector == cached_FAT_sector){ //the sector we need is cached
        return FAT_cache[(cluster%(bytes_per_sector/2))];//TODO: check this calc
    }
    else{ //we need to load a new sector
        load_sector(needed_sector,(void*) &FAT_cache);
        cached_FAT_sector = needed_sector;
        return FAT_cache[(cluster%(bytes_per_sector/2))];//TODO: check this calc
    }
}
/// @brief 
/// @return the first free cluster, 0 if there is none
uint16_t FAT_free(){
    for(uint16_t i =2;i<sectors_per_FAT*(bytes_per_sector/2);i++){
        if(!FAT_lookup(i)){return i;}
    }
    return 0;
}

int FAT_edit(uint16_t cluster,uint16_t val){//this is really inefficient but i dont have time nor energy for something better
    FAT_lookup(cluster); //it caches the cluster
    FAT_cache[(cluster%(bytes_per_sector/2))] = val;
    write_sector(FAT_start+(cluster*2)/bytes_per_sector,&FAT_cache);
    return 0; //TODO: Diagnostics
}


void get_volume(struct directory* ret){
    *ret = volume;
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
        dalloc((uintptr_t)ret,sizeof(directory_size));
    }
}





int write_sector(int sector_pos,void *memory_pos){
    dap.size = 0x10;
    dap.padding = 0;
    dap.num_of_sectors = 1;
    dap.offset = (uint16_t)(((unsigned int )memory_pos )%16);
    dap.segment = (uint16_t)(((unsigned int )memory_pos)/16);
    dap.sector = sector_pos;
    dap.rest =0;
    write_sector_helper(&dap);
    return 0;//TODO: Make actual diagnostics
}

//we are doing this so we can access the disk packet pos via the stack
int write_sector_helper(struct disk_address_packet *ptr){
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
        "mov ah, 0x43 \n "
        "mov al, 0x3 \n" //TODO: This is split among versions and idk what to put here
        "mov esi, dword [bp+8] ; +4 esp +4 size \n" 
        "ror esi, 4 \n "
        "mov ds, si \n "
        "shr esi, 28 \n "
        "int 0x13 \n "
        "jnc skipw \n"
        "mov bh, 0 \n"
        "mov al, 'f' \n"
        "mov ah, 0x0e \n"
        "int 0x10 \n"
        "skipw: \n"
        "pop ds \n "
        "popad \n "
    // ----------------------------------------------------------
    );
   

}


int load_sector(int sector_pos, void *memory_pos){ //idk if char pointer is good here
   dap.size = 0x10;
   dap.padding = 0;
   dap.num_of_sectors = 1;
   dap.offset = (uint16_t)(((unsigned int )memory_pos )%16);
   dap.segment = (uint16_t)(((unsigned int )memory_pos)/16);
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
        "push ds \n" //this breaks it for some fuckig reason, oh yeah i know just dont touch it
        "mov dl, 0x80  ; TODO make this flexible; \n"
        "xor ax, ax  \n "
        "mov ds, ax  \n "
        "mov ah, 0x42 \n "
        "mov esi, dword [bp+8] ; +4 esp +4 size \n" 
        "ror esi, 4 \n "
        "mov ds, si \n "
        "shr esi, 28 \n "
        "int 0x13 \n "
        "jnc skipl \n"
        "mov bh, 0 \n"
        "mov al, 'f' \n"
        "mov ah, 0x0e \n"
        "int 0x10 \n"
        "skipl: \n"
        "pop ds \n "
        "popad \n "
    // ----------------------------------------------------------
    );
   

}

/// @brief Idk how to name this
/// @param dir 
/// @param folder 
/// @return 
int delete_root(struct directory dir){
    
    size_t folder_size = root_dir_size*bytes_per_sector;
    struct directory* folder_listed = list_root();
    for(struct directory* i = folder_listed;i!=(struct directory*)(((char*)folder_listed)+folder_size);i++){
        if(cmp_name(i->name,dir.name)){
            *(i->name) = 0xE5;//this line doesn't work probbably
            delete_dir(*i);
            modify_root((char*)folder_listed);
            dalloc((uintptr_t)folder_listed,folder_size);
            return 0;
        }
    }
   
    return 1;
}

/// @brief Idk how to name this
/// @param dir 
/// @param folder 
/// @return 
int delete_(struct directory dir,struct directory folder){
    
    size_t folder_size = dir_size(folder);
    struct directory* folder_listed = list_dir(folder,folder_size);
    for(struct directory* i = folder_listed;i!=(struct directory*)(((char*)folder_listed)+folder_size);i++){
        if(cmp_name(i->name,dir.name)){
            *(i->name) = 0xE5;//this line doesn't work probbably
            delete_dir(*i);
            return 0;
        }
    }
    modify_dir(folder,(char*)folder_listed,folder_size);
    return 1;
}


/// @brief You cant delete the root dir,
/// @param dir directory and its sub-directories to be delted
/// @return success TODO
int delete_dir(struct directory dir){
    if(is_volume(dir)){return 1;}
    //now we have to mark it as deleted
    
    if(is_folder(dir)){
        size_t folder_size = dir_size(dir);
        struct directory* dir_listed = list_dir(dir,folder_size);
        for(struct directory* ptr = dir_listed;ptr->name[0]!=0;ptr++){
           
            if(ptr->name[0]!= '.' && ptr->name[0]!=0xE5){delete_dir(*ptr);}
        }
        dalloc((uintptr_t)dir_listed,folder_size);
    
    }
    
    //now we free up the current dir
    int cur_cluster = dir.starting_cluster;
    int next_cluster;
    while(1){
        next_cluster= FAT_lookup(cur_cluster);
        FAT_edit(cur_cluster,0);
        if(next_cluster>=0xFFF8){break;}
        cur_cluster = next_cluster;
    }
    return 0;
}

/// @brief mkdir    
/// @param name 
/// @param folder parameter folder of all folders
/// @return cluster where it's placed
uint16_t make_dir(char name[11], struct directory folder){
    struct directory* folder_stucture = (struct directory*)malloc(3*sizeof(directory_size));
    //clear the structs
    for(char* c = (char*)folder_stucture;c!=((char*)folder_stucture)+3*sizeof(directory_size);c++){*c=0;}
    //set up names 
    memcpy((void*)".          ",(void*)(folder_stucture[0].name),11);
    memcpy((void*)"..         ",(void*)(folder_stucture[1].name),11);
    folder_stucture[0].attribute = 0x10;
    folder_stucture[1].attribute = 0x10;
    struct directory maked_dir;
    for(char* c = (char*)&maked_dir;c!=((char*)&maked_dir)+sizeof(directory_size);c++){*c=0;}
    memcpy((void*)name,(void*)maked_dir.name,11);
    maked_dir.attribute = 0x10;
    uint16_t starting_cluster;
    uint16_t return_cluster;
    if(is_volume(folder)){
        starting_cluster = create_root(maked_dir);
        return_cluster =0;
    }
    else if(is_folder(folder)){
        starting_cluster = create_dir(maked_dir,folder);
        return_cluster = folder.starting_cluster;
    }
    else{
        return 1;
    }
    if(starting_cluster==1){return 1;}
    maked_dir.starting_cluster = starting_cluster; //this need's to be more elaborate when we start doing dates
    folder_stucture[0].starting_cluster = starting_cluster;
    folder_stucture[1].starting_cluster = return_cluster;
    modify_dir(maked_dir,(char*)folder_stucture,3*sizeof(directory_size));
    dalloc((uintptr_t)folder_stucture,3*sizeof(directory_size));
    //we are done
    return starting_cluster;
}


/// @brief This function is full of hacks and could easly break if something is changed, i dont really give a shit if it works
/// @param dir directory to be created, the only thing to be specified is the name and type
/// @param folder @folder
/// @return cluster where it's placed
uint16_t create_root(struct directory dir){//TODOO: check if dir name is valid
    
    // we want to allocate the first cluster
    dir.starting_cluster = FAT_free();
    FAT_edit(dir.starting_cluster, 0xFFFF); //per spec

    // prints("WE ARE IN CREATE DIR",21);
    // nl();
    size_t folder_size; 

    struct directory* base;
   
    folder_size = root_dir_size*bytes_per_sector;
    base = list_root();
    
    //we want to search for an empty space, if not we put it at the end and move a 0 to the end
    struct directory* free_space = NULL; 
    for(struct directory* ptr = base; ptr!= (struct directory*)(((char*)base)+root_dir_size*bytes_per_sector);ptr++){
        if(ptr->name[0]==0xE5||ptr->name[0]==0){//TODO: add support for changing 0x05 things
            free_space = ptr;
            break;
        }
    }
    
   
    *free_space = dir;
    modify_root((char*) base);
    dalloc((uintptr_t)base,folder_size);
    //nl();
    //dmph((char*)base,folder_size+2*sizeof(directory_size),16);
    //nl();
    
   
    //prints("RETURN OF THE DALLOC",21);
    //nl();
    return dir.starting_cluster;
}

/// @brief This function is full of hacks and could easly break if something is changed
/// @param dir directory to be created, the only thing to be specified is the name and type
/// @param folder @folder
/// @return starting cluster
uint16_t create_dir(struct directory dir,struct directory folder){//TODOO: check if dir name is valid
    if(!is_folder(folder)){return 1;}
    
    // we want to allocate the first cluster
    dir.starting_cluster = FAT_free();
    FAT_edit(dir.starting_cluster, 0xFFFF); //per spec

    // prints("WE ARE IN CREATE DIR",21);
    // nl();
    size_t folder_size; 

    struct directory* base;
    // prints("IS FOLDER yay",14);
    // nl();
    folder_size = dir_size(folder);
    // prints("RETURNED FROM THE questionable dir_size",40);//REMINDER: the bug is here, we have filed the first cluster of FOLDER1 and now dir size doesn't have an ending 0 to tell the size, to investigate we need to look at how this situation is handled usually
    // nl();
    base = list_dir(folder,folder_size+2*sizeof(directory_size)); //this is safe because we use the size only for malloc(which we want) and for saving the rest of the sector, which isn't a problem
    //we want to search for an empty space, if not we put it at the end and move a 0 to the end
    struct directory* free_space = NULL; 
    for(struct directory* ptr = base; ptr->name[0]!=0;ptr++){//here is the problem, it is not not always null ternimated
        if(ptr->name[0]==0xE5){//TODO: add support for changing 0x05 things
            free_space = ptr;
            break;
        }
    }
    
    if(free_space==NULL){//the folder is jam packed, we need to add a directory
        base[(folder_size)/(sizeof(directory_size))+1].name[0] = 0; //we edit the additional "hacked" directory so its the new end of file
        base[(folder_size)/(sizeof(directory_size))] = dir;
        //we hack the folder for modify_dir
        
        modify_dir(folder,(char*) base,folder_size+2*sizeof(directory_size));//0 at the end even if it means the whole cluster is just one 0
        dalloc((uintptr_t)base,folder_size+2*sizeof(directory_size));
        

    }
    else {
        *free_space = dir;
        modify_dir(folder,(char*) base,folder_size);
        dalloc((uintptr_t)base,folder_size);
    }
    //nl();
    //dmph((char*)base,folder_size+2*sizeof(directory_size),16);
    //nl();
    
   
    //prints("RETURN OF THE DALLOC",21);
    //nl();
    //this is another comment
    return dir.starting_cluster;
}

int modify_root(char* pos){
    
    for(int cur_sector = root_dir;cur_sector<root_dir+root_dir_size;cur_sector++){
        int t = write_sector(cur_sector,(void*)pos);
        if(t){return 1;}
        pos+=bytes_per_sector;
    }
    return 0;
}

/// @brief modifies an existing file
/// @dir the directory of the file, this function doesn't modify it
/// @pos
/// @return
int modify_dir(struct directory dir,char *pos,size_t size){ //this should be go, TODO : TAKE A LOOK BACK
   uint16_t cur_cluster = dir.starting_cluster; //the cluster we are currently at
   uint16_t next_cluster = FAT_lookup(cur_cluster); //the cluster which the current cluster is pointing to

    int cur_file_size_in_sectors = size/bytes_per_sector;
    if(size%512){cur_file_size_in_sectors++;}
    while(1){
        next_cluster = FAT_lookup(cur_cluster);
        if(next_cluster>=0xFFF8 && cur_file_size_in_sectors==1){//we are on our last sector for both
            printch('p');
            write_sector(data_start+(cur_cluster-2),pos); //write the last sector
            //DIAGNOSTICS

            break;
        }
        

        if(next_cluster>=0xFFF8){//we have reached the end of the previous file
            printch('q');
            while(cur_file_size_in_sectors>1){
                write_sector(data_start+(cur_cluster-2),pos);
                
                next_cluster = FAT_free();
                FAT_edit(cur_cluster,next_cluster);

                cur_cluster = next_cluster;
                pos+=bytes_per_sector;
                cur_file_size_in_sectors--;
            }
            write_sector(data_start+(cur_cluster-2),pos);
            FAT_edit(cur_cluster,0xFFFF);//per spec
           
            break;
        }

        if(cur_file_size_in_sectors<=0){
            printch('r');
            while(next_cluster<0xFFF8){
                next_cluster = FAT_lookup(cur_cluster);
                FAT_edit(cur_cluster,0);

            }
            break;
        }

        write_sector(data_start+(cur_cluster-2),pos); //where both are still running

        cur_cluster = next_cluster;
        cur_file_size_in_sectors--;
        pos+=bytes_per_sector;
    }
    return 0; //TODO: Diagnostics
   
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
/// @brief 
/// @param folder 
/// @return size of the directory in bytes 
size_t dir_size(struct directory folder){
    if(!is_folder(folder)){return 0;}
     //we are gonna hack it to find out the size 
    int cur_cluster = folder.starting_cluster;
    int next_cluster = FAT_lookup(cur_cluster);
    int cluster_count =0; 
    while(next_cluster<0xFFF8){
        cur_cluster= next_cluster;
        next_cluster = FAT_lookup(cur_cluster);
        cluster_count++;
        //load_sector(data_start+(cur_cluster-2),(void*) &temp_sector);

    }
    //then the current cluster points to the next one
    load_sector(data_start+(cur_cluster-2),(void*)&temp_sector);
    //dmph((char*)&temp_sector,512,16);
    //nl();
    //
    for(int i =0;i<bytes_per_sector;i+=32){
        if(!temp_sector[i]){//its the end of the file
            return cluster_count*bytes_per_sector+i;
        }
    }
    //if we have made it to here, means the last cluster is full, and invalid
    return 0;
    
}


/// @brief returnds a null terminated array of files and folders in the given directory, memory should be deallocated later
struct directory* list_dir(struct directory folder,size_t size){ //we already know the length of this list because its in the dir desc

   if(!is_folder(folder)){return NULL;}


   

    

    struct directory* result = (struct directory*) malloc(size); //this should be deallocated when done

    //we want to save the last part of the last sector
    
    //we have to hack the size here
    folder.file_size_in_bytes =  size;//this is a hack and should be illegal
    
    load_file(folder,(char*)result);

   
    return result;
}
struct directory* list_root(){
    struct directory* result = (struct directory*) malloc(root_dir_size*bytes_per_sector); //this should be deallocated when done
    char* ptr = (char*)result;
    
    for(int cur_sector = root_dir;cur_sector<root_dir+root_dir_size;cur_sector++){
        int t = load_sector(cur_sector,(void*)ptr); //chips one sector off
        ptr+=bytes_per_sector;
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
    if(!is_volume(folder)&&!is_folder(folder)){return NULL;}
    
    //first we load the folder into memory
    struct directory* search_place;
    size_t search_size;
    if(is_volume(folder)){//we search the root dir
        search_size =  root_dir_size*bytes_per_sector;
        search_place = list_root();
    }
    else{
        search_size= dir_size(folder);
        search_place= list_dir(folder,search_size);
    } 
    
    for(struct directory* i = search_place;i!=(struct directory*)(((char*)search_place)+search_size);i++){
        if(is_folder(*i) && i->name[0]!='.' && i->name[0]!=0xE5){//we have to check if its delted or if its .. or .
            
            dir_enqueue(*i);//if the queue overfills this doesn't work
        }
        
        if(cmp_name(name,i->name)){ //we have found the file yayy
            void* res = malloc(sizeof(directory_size));
            memcpy((void*)i,res,sizeof(directory_size));
            dalloc((uintptr_t)search_place,search_size); //we need to dealocate the memory, we don't want no leaks
            return (struct directory*) res;
        }

    }
    
    dalloc((uintptr_t)search_place,search_size); //we need to dealocate the memory, we don't want no leaks
    return NULL;

}



void start_program(){
    init();
    /*
    prints("STARTED FILE MANAGER TESTING",28);
    nl();
    //===========FILE MNG TESTING================
   
    struct directory* root_search = search_dir(volume,"FOLDER1    ");
    nl();
    printf((int)root_search);
    nl();
    prints("FOUND FOLDER PRESUMABLY",23);
    nl();

    //here we have to test listdir when its above 512
    prints("DUMPING MEMORY",14);
    print_mem();
    nl();
    size_t folder_size = dir_size(*root_search);
    printf((int)folder_size);
    nl();
   
    
    
    struct directory* listed_dir = list_dir(*root_search,folder_size);

    dmph((char*)listed_dir,folder_size,16);
    nl();
    struct directory folder1;
    struct directory dir1;
    for(char* c = (char*)&dir1;c<(((char*)&dir1)+32);c++){*c = 0;}
    memcpy(root_search,&folder1,sizeof(directory_size));
    
    
    memcpy((void*)"TEXTFI01TXT",&(dir1.name),11);
    create_dir(dir1,folder1);
    memcpy((void*)"TEXTFI02TXT",&(dir1.name),11);
    create_dir(dir1,folder1);
    memcpy((void*)"TEXTFI03TXT",&(dir1.name),11);
    create_dir(dir1,folder1);
    memcpy((void*)"TEXTFI04TXT",&(dir1.name),11);
    create_dir(dir1,folder1);
    memcpy((void*)"TEXTFI05TXT",&(dir1.name),11);
    create_dir(dir1,folder1);
    memcpy((void*)"TEXTFI06TXT",&(dir1.name),11);
    create_dir(dir1,folder1);
    memcpy((void*)"TEXTFI07TXT",&(dir1.name),11);
    create_dir(dir1,folder1);
    memcpy((void*)"TEXTFI08TXT",&(dir1.name),11);
    create_dir(dir1,folder1);
    memcpy((void*)"TEXTFI09TXT",&(dir1.name),11);
    create_dir(dir1,folder1);
    memcpy((void*)"TEXTFI10TXT",&(dir1.name),11);
    create_dir(dir1,folder1);
    memcpy((void*)"TEXTFI11TXT",&(dir1.name),11);
    create_dir(dir1,folder1);
    memcpy((void*)"TEXTFI12TXT",&(dir1.name),11);
    create_dir(dir1,folder1);
    memcpy((void*)"TEXTFI13TXT",&(dir1.name),11);
    create_dir(dir1,folder1);
    memcpy((void*)"TEXTFI14TXT",&(dir1.name),11);
    create_dir(dir1,folder1);
    memcpy((void*)"TEXTFI15TXT",&(dir1.name),11);
    create_dir(dir1,folder1);
    memcpy((void*)"TEXTFI16TXT",&(dir1.name),11);
    create_dir(dir1,folder1);
    memcpy((void*)"TEXTFI17TXT",&(dir1.name),11);
    create_dir(dir1,folder1);
    memcpy((void*)"TEXTFI18TXT",&(dir1.name),11);
    create_dir(dir1,folder1);
    memcpy((void*)"TEXTFI19TXT",&(dir1.name),11);
    create_dir(dir1,folder1);
    memcpy((void*)"TEXTFI20TXT",&(dir1.name),11);
    create_dir(dir1,folder1);


    
    //we are gonna print memory here
    prints("PRINTING MEMORY AFTER ADDING THE FILES AND ABUSING THE MEMMNG",61);
    nl();
    print_mem();
    nl();
    folder_size = dir_size(*root_search);//dir size fucks with us all
    printf((int)folder_size);
    nl();
    listed_dir = list_dir(*root_search,folder_size);
  
    
    dmph((char*)listed_dir,folder_size,16);
    nl();





    make_dir("MFOLDER    ",volume);
    


    
    prints("STARTED DELETE TESTING",22);
    nl();

    delete_root(*root_search);


    */
}

void init(){
    //we want to setup the root dir
    char temp_volume_name[11] = "BORISOSVOL"; //this changes with volume_label
    memcpy(&temp_volume_name,&volume.name,11);
    volume.attribute = 1<<3;
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
    return dir.attribute & 1;
}

int is_hidden(struct directory dir){
    return dir.attribute & (1<<1);
}

int is_system(struct directory dir){
    return dir.attribute & (1<<2);
}

int is_volume(struct directory dir){
    return dir.attribute & (1<<3);
}

int is_folder(struct directory dir){
    return dir.attribute & (1<<4);
}

int is_archive(struct directory dir){
    return dir.attribute & (1<<5);
}

