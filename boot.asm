[org 0x7c00]



global start

;;========================Variables for the FAT16 file system==========================
%assign jump_point 0x3E
%define OEM_ID 'BSD  4.4' 
%assign bytes_per_sector 512
;in code we assume this is always 1
%assign sectors_per_clutser 1 
%assign reserved_sectors 1
%assign num_of_FATs 1
;each directory is 32
;;please make it divisible by 16
%assign num_of_root_dir 512 
%assign small_sector_num 0xFFFF
%assign media_descriptor 0xF8
;;this should be calcualed according to small_sector_num
%assign sectors_per_FAT 61
%assign sectors_per_track 0
%assign num_of_heads 1
;;start of volume basically
%assign hidden_sectors 0
%assign large_sector_num 0
%assign drive_num 0x80
%assign reserved 0
%assign extended_boot_sig 0x29
;;idk what this is MacOS put it there
%assign volume_serial_num 0x93811B4
;should be the same as the root dir
%define volume_label 'BORISOSVOL '
%define file_system_type 'FAT16   '
;;-----------------------------------
;;Calculated starting points, in sectors
%assign volume_start hidden_sectors
%assign FAT_start volume_start + reserved_sectors 
%assign root_dir FAT_start+num_of_FATs*sectors_per_FAT
%assign root_dir_size (num_of_root_dir*32)/512
;;--------------------------------------------------------
;;assign where to put the sectors
%assign search_sector_address 0x7E00
%assign disk_packet_struct 0x8000
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

  
   xor ax, ax
   


    xor ax,ax
    mov ds, ax
    mov ss,ax
    mov sp, 0x9c00 ; this is arbitrary and the stack could be whatever it wants??
    
    
    cli 
    push ds

    lgdt [gdtinfo]

    mov eax, cr0
    or al, 1
    mov cr0, eax
    jmp pmode

pmode:
    mov bx, 0x10
    mov ds, bx

    and al, 0xFE
    mov cr0,eax
    jmp unreal

unreal:
    pop ds
    sti


    ;;here the code starts, we are in unral mode and everything is okay
    ;;we need to find loader.sys which should be in the root dir
    ;------------------------we check if we have int 13h extentions-------------------------
        mov ah, 0x41
        mov bx,0x55AA
        mov dl, 0x80 ;; TODO: ged the actual reading from DL, from the bios
        int 0x13
        jc no_int13_ext ;;else we are good and do not need to print anything



    ;--------------------------------------------------------------------------------------

    ;-----------------------we set up the loop---------------------------
    mov edx, root_dir; this tracts the sector of the drive in which we are searching
    mov bx, search_sector_address ; this is the offset inside the sector in MEMORY
    add bx, bytes_per_sector ; to set up the loop propertly so it reads a sector immedietly

    
    
    ;-------------------------------------------------------------------
    loop:
 
        cmp edx, (root_dir + root_dir_size)
        jge end

        cmp bx, (bytes_per_sector+search_sector_address) ;;check if we need to load a new sector
        jb no_new_sector 
        ;-----------------------------we have to load the new sector--------------------------------

        
        mov byte [disk_packet_struct], 0x10 ;size of packet is 16 bytes
        mov byte [disk_packet_struct+1],0 ; always 0
        mov word [disk_packet_struct+2],1 ; number of sectors to transfer
        mov word [disk_packet_struct+4], (search_sector_address) ;offset of placement
        mov word [disk_packet_struct+6], 0 ;segment of placement
        mov dword [disk_packet_struct+8] , edx ; this is in sectors
        mov dword [disk_packet_struct+12],0 ; should a word or a dword be here?? i have no clue, because its 32 bit i think its word but whatever

            ;----------------------We call int 13h-----------------------------
            pusha
            mov dl, 0x80 ;;TODO make this flexible
            xor ax, ax
            mov ds, ax
            mov ah, 0x42
            mov si, disk_packet_struct
            int 0x13
            popa
            ;---------------------------------------------------------
            
        

        mov bx, search_sector_address ;; we reset bx
        add edx, 1 ;;increment dx
        ;----------------------------------------------------------------------------------------
        no_new_sector:
        
        
        
        



        jmp is_loader


        found_loader: ; congrats we have found the loader
            ;;load it into memory;;TODO
            jmp load_loader

        no_loader_found:
        
        add bx,32

    jmp loop
    end: ; we assume no loader foudnd
    mov ah, 0x0E
	mov bh, 0x00

    ;we shall unroll the code for space reasons
    mov al, 'N'
    int 10h
    mov al, 'O'
    int 10h
    mov al, 'L'
    int 10h
    mov al, 'O'
    int 10h
    mov al, 'A'
    int 10h
    mov al, 'D'
    int 10h
    mov al, 'E'
    int 10h
    mov al, 'R'
    int 10h


jmp $

load_loader:
 
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


;we go here if int 13 ext is not supported
no_int13_ext:
    mov ah, 0x0E
	mov bh, 0x00

    ;we shall unroll the code for space reasons
    mov al, 'O'
	int 10h
    mov al, 'N'
	int 10h
	mov al, 'I'
	int 10h
    mov al, 'N'
	int 10h
    mov al, 'T'
	int 10h
    mov al, '1'
	int 10h
    mov al, '3'
	int 10h
    mov al, 'E'
	int 10h
    mov al, 'X'
	int 10h
    mov al, 'T'
	int 10h

jmp $;; TODO Make this say press enter to continue 

hprint:
    push bp,
    mov bp,sp

    mov bx, 4 ;number of nibbles!
    .loop:
    cmp bx, 0
    jz .end
    dec bx
    mov ax, 4
    mov cx, 3
    sub cx, bx
    mul cx

    mov cx, ax
    mov ax, word [bp+4] ;hex value argument

    shl ax, cl
    shr ax, 12
     
    cmp al, 10 
    jl .num
    mov ah, 55
    jmp .char
    .num:
    mov ah, 48
    .char:
    add al,ah
    mov ah, 0x0e
    int 0x10
    jmp .loop
    .end:

    mov ah, 0x0e
    mov al, ' '
    int 0x10

    pop bp
ret 2

gdtinfo:
   dw gdt_end - gdt - 1   ;last byte in table
   dd gdt                 ;start of table

gdt:        dd 0,0        ; entry 0 is always unused
codedesc:   db 0xff, 0xff, 0, 0, 0, 10011010b, 00000000b, 0
flatdesc:   db 0xff, 0xff, 0, 0, 0, 10010010b, 11001111b, 0
gdt_end:
 
times 510-($-$$) db 0
dw 0xaa55

;;==========================================FAT TABLE================================

db 0x80
db 0xFF

db 0xFF ;; end of file entry 
db 0xFF



times 508 db 0 ;;512 - the four bytes already defined

times (num_of_FATs*sectors_per_FAT-1)*512 db 0 ;;size of fat -1 because we already defined the first sector

;;======================ROOT DIR=======================
db volume_label
db 0x08
times 20 db 0


;;=================FAT TESTING CODE=================== REMOVE LATER
db 'FOLDER1    '
db 0x10
times 14 db 0
dw 2 ; this is a word for the cluster ; its should be data start + 1kb
dd 0


;;first we get out of the root dir

times 32*(512-2) db 0

;;then we skip the first 2 sectors

times 512*2 db 0 ; this should be edited when code above is changed
;;prev

db '.          '
db 0x10
times 14 db 0
dw 2
dd 0

db '..         '
db 0x10
times 14 db 0
dw 0
dd 0


times 1000 db 0