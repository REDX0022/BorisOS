asm("db '</FILE NAME/>'");
asm("times 5 db");
asm("times 16 db"); //null terminated

extern void _start_program();

//extern void init_</FILE NAME WITHOUT EXT/>(void* funcs);

void __start__(void** libs){
    //call externs above
    _start_program();
}