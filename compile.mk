kernel_libs := LIBS/c0du.asm 
targets := LOADER.SYS MEMMNG.SYS transfer

run: $(targets)

MEMMNG.SYS: OS/MEMMNG.C $(kernel_libs)
	smlrcc -unreal LIBS/c0du.asm SRC/MEMMNG/MEMMNG.C -o OS/MEMMNG.SYS -map MEMMNG.PMA 

LOADER.SYS: /OS/LOADER.C $(kernel_libs)
	smlrcc -unreal LIBS/c0du.asm SRC/LOADER.C -o OS/LOADER.SYS -I LIBS/

transfer:
	git commit -am Debug 
	git push

/OS/LOADER.C:

/OS/MEMMNG.C:





