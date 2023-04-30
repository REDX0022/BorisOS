ref 1:Memory map when the BIOS is loadead
https://wiki.osdev.org/Memory_Map_(x86)#Overview

ref 2:List of BIOS interrupts
https://wiki.osdev.org/Ralf_Brown%27s_Interrupt_List
http://www.ctyme.com/intr/

ref 3:Protected mode to unlock full cpu power - interrupts arent allowed here!
https://en.wikibooks.org/wiki/X86_Assembly/Protected_Mode

ref 3: Far pointers in C
https://www.geeksforgeeks.org/what-are-near-far-and-huge-pointers/

ref 4: Good example (done in unreal mode)
https://github.com/fysnet/FYSOS/tree/master/loader

ref 5: Unreal mode
https://en.wikipedia.org/wiki/Unreal_mode


ref 6: Lighter bootloader
https://wiki.osdev.org/BootProg

ref 7: DOS MZ file format (-unreal complies to this)
http://justsolve.archiveteam.org/wiki/MS-DOS_EXE

ref 8: Good textbook man(TU Munchen)
https://csc-knu.github.io/sys-prog/books/Andrew%20S.%20Tanenbaum%20-%20Modern%20Operating%20Systems.pdf

ref 9: Best Fat16 refrence
http://www.maverick-os.dk/FileSystemFormats/FAT16_FileSystem.html


ref10: Apperantly to get a working drive you need a partition table man wth
https://en.wikipedia.org/wiki/Master_boot_record#PTE

## HOW SHOULD AN OS BE DESIGNED ##

So we boot the bootloader
    -first we have the boot man 
    -then the loader
    **Early loader**
    Doesn't have syscalls
    Has only rudementary debug 

    **Late loader**
    Has sys calls
    -we have executables and dll's

**We dont use interrupts for syscalls man thats dumb**

Next we run the memory managment
    -we run in unreal mode so the segment calls are effective
    -we shall have small malloc because then we can fuck with it
    -we shall do far system calls
    -**THIS IS FUCKING IMPORTANT**
        -in unreal mode when you compile you have to include __c0du.asm__ and __irq5isr.c__ 
        -this bakes in the programm to fix the dynamically alocated address space
        -i thing we should put this in the programm loader it seems more efficient that way
        - the modifications should be simpe enough
The memory manager runs then the next thing which is the system calls operator is run ig???




We need a scheduler 
    -how do we do a scheduler


## WE NEED OUR EXECUTABLE FILE .BEX ##

First its just requests for library location
Yes thats cool but what the fuck do we do when we need to fuck with this
SO the problem is the stack
ITS GLOBAL so far call funciton for sys calls wont work 

