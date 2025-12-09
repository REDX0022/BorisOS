### A SMALL OPERATING SYSTEM ###
**Because of FAT16 limitations all flies and folcers should be named with UPPERCASE**

## BOOT ##
- E on the screen means there are not int 13h bios extentions
- NL - || - no loader has been found
- HERE YOU CAN FIND THE ORIGINIAL DRIVE PARAMETERS


## LOADER ##
- The early loader 
    - doesnt have any syscalls
    - loads only from the rootdir
    - doesnt look which syscalls are needed



## SHARED LIBRARIES FILE STRUCTURE ##
- At the beggining of the file there is an extension to the MZ header
- Next to shared libraries there is a .MAP file which will contain function offsets
    - The first 16 bit unsigned int is the number of functions
    - Next are the function offsets, all 16 bit ** unsigned **
    - MAP size limited to one sector

## STACK MANAGMENT ##
-when we switch contexts we save the sp of the previous stack at the bottom of the new one
-we shall have a syscall for context swiching?

## Strings ##
- null terminated


## How to run ## 
i have forgot how to turn on this os
- mount a vhdx to wsl
- run wsl
- run lin2.mk 

## Thing needed to run ##
- wsl 2
- Qemu
- a hobby compiler SmallerC, it should be in the PATH variable
