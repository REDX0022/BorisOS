prereq := BOOT/boot.o OS/LOADER.SYS OS/MEMMNG.SYS OS/MEMMNG.MAP 

#Get-CimInstance -ClassName Win32_DiskDrive | Format-List -Property DeviceID,BytesPerSector,Index,Caption,InterfaceType,Size,TotalSectors,SerialNumber
run: $(prereq)
	
	
	
	-sudo umount /dev/sdd
	
	
	sudo dd if=BOOT/boot.o of=/dev/sdd

	sudo mount /dev/sdd /BORISOSVOL

	nasm SRC/MEMMNG/MEMMNGMAP.asm -o OS/MEMMNG.MAP
	nasm SRC/FILEMNG/FILEMNGMAP.asm -o OS/FILEMNG.MAP

	sudo cp OS/LOADER.SYS /BORISOSVOL
	sudo cp OS/MEMMNG.SYS /BORISOSVOL
	sudo cp OS/MEMMNG.MAP /BORISOSVOL
	sudo cp OS/FILEMNG.SYS /BORISOSVOL
	
	sudo umount /dev/sdd
	
	sudo qemu-system-x86_64 -cpu qemu64 -drive format=raw,file=/dev/sdd -nographic
	
	
	

BOOT/boot.o:
OS/LOADER.SYS:
OS/MEMMNG.SYS:
OS/MEMMNG.MAP:
OS/FILEMNG.SYS:
OS/FILEMNG.MAP: