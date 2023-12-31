asm("db '</MAP NAME/>'");
asm("times 5 db 0");
asm("times 16 db 0"); //null terminated

extern void _start_program();

//extern void init_</FILE NAME WITHOUT EXT/>(void* funcs);

void __start__(void** libs){
    //call externs above
    _start_program();
}