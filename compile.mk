kernel_libs := LIBS/c0du.asm 
targets := OS/LOADER.SYS OS/MEMMNG.SYS OS/FILEMNG.SYS transfer

run: $(targets)

BOOT/boot.o:
	nasm BOOT/boot.asm -o BOOT/boot.o

OS/MEMMNG.SYS: SRC/MEMMNG.C $(kernel_libs)
	smlrcc -unreal SRC/MEMMNG/linked_header_MEMMNG.c LIBS/c0du.asm SRC/MEMMNG/MEMMNG.C -o OS/MEMMNG.SYS -map SRC/MEMMNG/MEMMNG.PMA  -I LIBS/

OS/LOADER.SYS: SRC/LOADER.C $(kernel_libs)
	smlrcc -unreal  LIBS/c0du.asm SRC/LOADER.C -o OS/LOADER.SYS -I LIBS/

OS/FILEMNG.SYS:
	smlrcc -unreal SRC/FILEMNG/linked_header_FILEMNG.c LIBS/c0du.asm SRC/FILEMNG/FILEMNG.C -o OS/FILEMNG.SYS -map SRC/FILEMNG/FILEMNG.PMA  -I LIBS/


transfer:
	git commit -am Debug 
	git push

SRC/LOADER.C:

SRC/MEMMNG.C:





