asm("db 'MEMMNG  SYS'");
asm("times 5 db 0");
asm("times 16 db 0"); //null terminated


extern void start_program();

extern void init_MEMMNG(void* funcs);

void __start__(void** libs){
    init_MEMMNG(libs[0]);
    start_program();
}