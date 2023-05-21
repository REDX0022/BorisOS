#include <stdint.h>
#include <stddef.h>


void printf(int n){
    if(n==0){printch('0');return;}
    printf(n/10);
    char tmpprint =((uint32_t)n%10)+48;
    printch(tmpprint&0xFF); //48 is the offset of the char 0
    n/=10;
    
}
void nl(){
    printch(0xA);
    printch(0xD);
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

char hex[17] = "0123456789ABCDEF";
void dmph(char* ptr, size_t len,int bytes_per_row){
    int j = 0;
    for(int i =0;i<len;i++){
        char high = (char)(hex[((*ptr)&0xFF)>>4]);
        char low = (char)(hex[(*ptr)&0xF]);

        printch(high);
        printch(low);
        printch(' ');
        if(j==bytes_per_row){
            j =0;
            nl();
        }
        ptr++;
        j++;
    }
    
}







