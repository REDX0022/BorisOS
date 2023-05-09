### THIS SHOULD BE AN OPERATING SYSTEM ###
**Because of FAT16 limitations all flies and folcers should be named with UPPERCASE**

## BOOT ##
- E on the screen means there are not int 13h bios extentions
- NL - || - no loader has been found
- HERE YOU CAN FIND THE ORIGINIAL DRIVE PARAMETERS

## STACK MANAGMENT##
-when we switch contexts we save the sp of the previous stack at the bottom of the new one
-we shall have a syscall for context swiching

## Strings##
- null terminated