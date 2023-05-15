kernel_libs := LIBS/c0du.asm LIBS/irq5isr.c
targets := LOADER.SYS MEMMNG.SYS transfer

run: $(targets)

MEMMNG.SYS: OS/MEMMNG.C $(kernel_libs)
	smlrcc -unreal LIBS/c0du.asm LIBS/irq5isr.c OS/MEMMNG.C -o MEMMNG.SYS -map MEMMNG.MAP

LOADER.SYS: /OS/LOADER.C $(kernel_libs)
	smlrcc -unreal LIBS/c0du.asm LIBS/irq5isr.c OS/LOADER.C -o LOADER.SYS

transfer:
	git commit -am Debug 
	git push

/OS/LOADER.C:

/OS/MEMMNG.C:





