nasm boot.asm -o boot.o



sudo dd if=boot.o of=/dev/disk4

diskutil mountDisk disk4

cp LOADER.SYS /Volumes/BORISOSVOL/

diskutil unmountDisk disk4

sudo qemu-system-x86_64 -cpu qemu64 -drive format=raw,file=/dev/disk4 -nographic