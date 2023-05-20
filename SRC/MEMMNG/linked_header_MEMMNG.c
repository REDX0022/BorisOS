asm("times 16 db"); //null terminated

extern void start_program();

//extern void init_</FILE NAME WITHOUT EXT/>(void* funcs);

void __start__(void** libs){
    //call externs above
    start_program();
}