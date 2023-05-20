prereq := BOOT/boot.asm OS/LOADER.SYS OS/MEMMNG.SYS OS/MEMMNG.MAP


run: $(prereq)
	git pull
	nasm BOOT/boot.asm -o BOOT/boot.o
	diskutil unmountDisk disk4
	sudo dd if=BOOT/boot.o of=/dev/disk4
	diskutil mountDisk disk4

	nasm SRC/MEMMNG/MEMMNGMAP.asm -o OS/MEMMNG.MAP

	cp OS/LOADER.SYS /Volumes/BORISOSVOL/
	cp OS/MEMMNG.SYS /Volumes/BORISOSVOL/
	cp OS/MEMMNG.MAP /Volumes/BORISOSVOL/
	diskutil unmountDisk disk4
	sudo qemu-system-x86_64 -cpu qemu64 -drive format=raw,file=/dev/disk4 -nographic

boot.asm:
OS/LOADER.SYS:
OS/MEMMNG.SYS:
OS/MEMMNG.MAP:


p: