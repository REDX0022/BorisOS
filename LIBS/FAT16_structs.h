#include "stddef.h"
#include "stdint.h"


struct disk_address_packet{
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