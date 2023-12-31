/*Here we want to make a decent shell programm*/




//lets use a lot of interrupts
#include "stddef.h"
#include "stdint.h"

//==========================DEFINTIONS==========================
#define VGA_MODE 0x03
#define WIDTH 80
#define HEIGHT 25
#define BUFFER_HEIGHT 1000//scroll feature TODO
#define MEMORY_BUFFER 0xB8000




char*  welcome_message= "Welcome to BorisOS 1.0";


uint16_t buffer[WIDTH][HEIGHT];


void specify_video_mode(){//we are on video mode 3 for now
    asm("push ax \n"
        "mov ax, 0x0003 \n"    
        "int 0x10 \n"
        "pop ax \n"
    );
}

void set_cursor_size(){
    asm("pushad \n"
        "mov ah, 1 \n" //this is an interrupt parametere
        "mov ch, 0b00000001 \n" //this specifies the type of cursor and size
        "mov cl, 0x00001110 \n" //specifies the size, see int 10h ah 1 for details
        "int 0x10 \n"    
        "popad \n"    
    );
}

/// @brief Sets the cursos position
/// @param row from top
/// @param column from left
/// @param page_number must be between 0 and 7 
/// @return success
int set_cursor_position(unsigned char row, unsigned char column, unsigned char page_number){
    if(page_number>7){return 1;}
    asm("pushad \n"
        "mov ah, 0x02 \n"
        "mov bh, byte [bp+16]\n" //this is the page number
        "mov dh, byte [bp+8] \n" //row
        "mov dl, byte [bp+4] \n" //column
        "int 0x10 \n"
        "popad \n"
    );
    return 0;
}

/// @brief Selects which page to display=
/// @param page_number 
/// @return 
int select_active_page(unsigned char page_number){
    if(page_number>7){return 1;}
    asm("pushad \n"
        "mov ah, 0x05 \n"
        "mov al, byte [bp+8] \n"
        "int 0x10 \n"
        "popad \n"
    );
    return 0;
}

int scroll_up(){
    asm("pushad \n"
        "mov ah, 0x06 \n"
        "mov al, 0x01 \n" //amount of lines to scroll up
        "int 0x10 \n"
        "popad \n"
    );
    return 0;
}


void write_char(int x, int y, unsigned char c){//it will be black and white for now
    uint16_t* address = (uint16_t*)(MEMORY_BUFFER+(y*WIDTH)+x);
    *address =  c | (0x01<<8);
}


void clear(){
    for(int i =0;i<WIDTH; i++){
        for(int j =0;j<HEIGHT; j++)
        {
            write_char(i,j,0);
        }
    }
}

void init(){
    //specify_video_mode();
    //clear();

}


void start_program(){
    init();
    for(int i =0;i<25;i++){
        write_char(i,1,welcome_message[i]);
    }
   
}




