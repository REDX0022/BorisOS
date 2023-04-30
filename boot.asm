[org 0x7c00]

global start



;;variables for the fat16 file system
%define jump_point 0x3C
%define OEM_ID 'BSD  4.4' 
%define bytes_per_sector 512
%define sectors_per_clutser 1
%define reserved_sectors 1
%define num_of_FATs 1
;each directory is 32
;;please make it divisible by 16
%define num_of_root_dir 512 
%define small_sector_num 0xFFFF
%define media_descriptor 0xF8
;;this should be calcualed according to small_sector_num
%define sectors_per_FAT 61
%define sectors_per_track 0
%define num_of_heads 1
;;start of volume basically
%define hidden_sectors 0
%define large_sector_num 0
%define drive_num 0x80
%define reserved 0
%define extended_boot_sig 0x29
;;idk what this is MacOS put it there
%define volume_serial_num 0x93811B4
;should be the same as the root dir
%define volume_label 'BORISOSVOL '
%define file_system_type 'FAT16   '
;;-----------------------------------
;;Calculated starting points, in sectors
%define volume_start reserved_sectors
%define FAT_start volume_start + reserved_sectors 
%define root_dir FAT_start+num_of_FATs*sectors_per_FAT
%define root_dir_size (num_of_root_dir*32)/512;
;;--------------------------------------------------------
;;Define where to put the sectors
%define search_sector_address 0x7E00
;;============================================================
;;now we define the bytes for the file system
;;jump point
db 0xEB
db jump_point
db 0x90
;;os name
db OEM_ID
;;bios parameter block
dw bytes_per_sector
db sectors_per_clutser
dw reserved_sectors
db num_of_FATs
dw num_of_root_dir
dw small_sector_num
db media_descriptor
dw sectors_per_FAT
dw sectors_per_track
dw num_of_heads
dd hidden_sectors
dd large_sector_num
;;extended bios parameter block
db drive_num
db reserved
db extended_boot_sig
dd volume_serial_num
db volume_label
db file_system_type




;
start:
    xor ax,ax
    xor ds, ds
    xor ss,ss
    mov sp, 0x9c00 ; this is arbitrary and the stack could be whatever it wants??
    
    cli 
    push ds
    lgdt [gdtinfo]

    mov eax, cr0
    or al, 1
    mov cr0, eax
    jmp 0x8:pmode

pmode:
    mov bx, 0x10
    mov ds, bx

    and al, 0xFE
    mov cr0,eax
    jmp 0x0:unreal

unreal:
    pop ds
    sti
    mov bx,

    ;;here the code starts, we are in unral mode and everything is okay
    ;;we need to find loader.sys which should be in the root dir


    ;-----------------------we set up the loop---------------------------
    mov dx, root_dir
    mov bx, search_sector_address
    add bx, 512

    
    ;-------------------------------------------------------------------
    .loop:
        cmp bx, (512+search_sector_address)
        jl .no_new_sector 
        ;-----------------------------we have to load the new sector--------------------------------

        ;;TODO Check if int 13h extensions are available
        ;;and if they are search proparty man
         mov ah, 0x02
        mov al, 1
        mov ch,0
    
        mov dh,0
        mov cl,2
        mov dl, 0x80
        mov bx, search_sector_address
        int 0x13




        ;----------------------------------------------------------------------------------------
        .no_new_sector:
        jmp is_loader


        found_loader: ; congrats we have found the loader
            ;;load it into memory;;TODO

        no_loader_found:
        ;;----------------------------we go to the next 
        add bx,32


jmp $

;;we have it in memory at bx
is_loader:
    cmp byte [bx],'L'
    jne no_loader_found

    cmp byte [bx+1],'O'
    jne no_loader_found

    cmp byte [bx+2],'A'
    jne no_loader_found

    cmp byte [bx+3],'D'
    jne no_loader_found

    cmp byte [bx+4],'E'
    jne no_loader_found

    cmp byte [bx+5],'R'
    jne no_loader_found

    cmp byte [bx+6],' '
    jne no_loader_found

    cmp byte [bx+7],' '
    jne no_loader_found

    cmp byte [bx+8],'S'
    jne no_loader_found

    cmp byte [bx+9],'Y'
    jne no_loader_found

    cmp byte [bx+10],'S'
    jne no_loader_found

    jmp found_loader


gdtinfo:
   dw gdt_end - gdt - 1   ;last byte in table
   dd gdt                 ;start of table

gdt:        dd 0,0        ; entry 0 is always unused
codedesc:   db 0xff, 0xff, 0, 0, 0, 10011010b, 00000000b, 0
flatdesc:   db 0xff, 0xff, 0, 0, 0, 10010010b, 11001111b, 0
gdt_end:
 

times 510-($-$$) db 0
dw 0xaa55