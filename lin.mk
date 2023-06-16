prereq := BOOT/boot.o OS/LOADER.SYS OS/MEMMNG.SYS OS/MEMMNG.MAP  OS/FILEMNG.SYS
kernel_libs := LIBS/c0du.asm LIBS/kernel_out.h SRC/MEMMNG/linked_header_MEMMNG.c
targets := OS/LOADER.SYS OS/MEMMNG.SYS OS/FILEMNG.SYS 


run: $(prereq)
	
	-sudo umount /dev/sdc
	-sudo dd if=/dev/zero of=/dev/sdc
	
	
	sudo dd if=BOOT/boot.o of=/dev/sdc

	sudo mount /dev/sdc /BORISOSVOL




	sudo cp OS/LOADER.SYS /BORISOSVOL
	sudo cp OS/MEMMNG.SYS /BORISOSVOL
	sudo cp OS/FILEMNG.SYS /BORISOSVOL
	sudo cp OS/MEMMNG.MAP /BORISOSVOL

	sudo umount /dev/sdc
	
	sudo qemu-system-x86_64 -cpu qemu64 -drive format=raw,file=/dev/sdc -nographic
	
	
	
	
BOOT/boot.o: BOOT/boot.asm
	nasm BOOT/boot.asm -o BOOT/boot.o

OS/MEMMNG.SYS: SRC/MEMMNG.C $(kernel_libs)
	smlrcc -unreal SRC/MEMMNG/linked_header_MEMMNG.c LIBS/c0du.asm SRC/MEMMNG/MEMMNG.C -o OS/MEMMNG.SYS -map SRC/MEMMNG/MEMMNG.PMA  -I LIBS/

OS/LOADER.SYS: SRC/LOADER.C $(kernel_libs)
	smlrcc -unreal  LIBS/c0du.asm SRC/LOADER.C -o OS/LOADER.SYS -I LIBS/

OS/FILEMNG.SYS: SRC/FILEMNG.C SRC/FILEMNG/linked_header_FILEMNG.c
	smlrcc -unreal SRC/FILEMNG/linked_header_FILEMNG.c LIBS/c0du.asm SRC/FILEMNG/FILEMNG.C -o OS/FILEMNG.SYS -map SRC/FILEMNG/FILEMNG.PMA  -I LIBS/

OS/MEMMNG.MAP: SRC/MEMMNG/MEMMNGMAP.asm
	nasm SRC/MEMMNG/MEMMNGMAP.asm -o OS/MEMMNG.MAP
OS/FILEMNG.MAP: SRC/FILEMNG/FILEMNGMAP.asm
	nasm SRC/FILEMNG/FILEMNGMAP.asm -o OS/FILEMNG.MAP


SRC/LOADER.C:


SRC/MEMMNG.C:


SRC/FILEMNG.C:

SRC/MEMMNG/MEMMNGMAP.asm:

SRC/FILEMNG/FILEMNGMAP.asm:

BOOT/boot.asm:

