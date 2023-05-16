prereq := boot.asm LOADER.SYS MEMMNG.SYS


run: $(prereq)
	git pull
	nasm boot.asm -o boot.o
	diskutil unmountDisk disk4
	sudo dd if=boot.o of=/dev/disk4
	diskutil mountDisk disk4
	cp LOADER.SYS /Volumes/BORISOSVOL/
	cp MEMMNG.SYS /Volumes/BORISOSVOL/
	diskutil unmountDisk disk4
	sudo qemu-system-x86_64 -cpu qemu64 -drive format=raw,file=/dev/disk4 -nographic

boot.asm:
LOADER.SYS:
MEMMNG.SYS:

p: