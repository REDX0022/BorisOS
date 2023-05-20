kernel_libs := LIBS/c0du.asm 
targets := OS/LOADER.SYS OS/MEMMNG.SYS transfer

run: $(targets)

OS/MEMMNG.SYS: SRC/MEMMNG.C $(kernel_libs)
	smlrcc -unreal LIBS/c0du.asm SRC/MEMMNG/MEMMNG.C -o OS/MEMMNG.SYS -map MEMMNG.PMA 

OS/LOADER.SYS: SRC/LOADER.C $(kernel_libs)
	smlrcc -unreal LIBS/c0du.asm SRC/LOADER.C -o OS/LOADER.SYS -I LIBS/

transfer:
	git commit -am Debug 
	git push

SRC/LOADER.C:

SRC/MEMMNG.C:





