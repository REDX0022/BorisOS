prereq := BOOT/boot.o OS/LOADER.SYS OS/MEMMNG.SYS OS/MEMMNG.MAP 


run: $(prereq)
	git pull
	
	diskutil unmountDisk disk4
	sudo dd if=BOOT/boot.o of=/dev/disk4
	diskutil mountDisk disk4

	nasm SRC/MEMMNG/MEMMNGMAP.asm -o OS/MEMMNG.MAP
	nasm SRC/FILEMNG/FILEMNGMAP.asm -o OS/FILEMNG.MAP

	cp OS/LOADER.SYS /Volumes/BORISOSVOL/
	cp OS/MEMMNG.SYS /Volumes/BORISOSVOL/
	cp OS/MEMMNG.MAP /Volumes/BORISOSVOL/
	cp OS/FILEMNG.SYS /Volumes/BORISOSVOL/
	
	
	
	
	
	diskutil unmountDisk disk4
	sudo qemu-system-x86_64 -cpu qemu64 -drive format=raw,file=/dev/disk4 -nographic

BOOT/boot.o:
OS/LOADER.SYS:
OS/MEMMNG.SYS:
OS/MEMMNG.MAP:
OS/FILEMNG.SYS:
OS/FILEMNG.MAP:


p: