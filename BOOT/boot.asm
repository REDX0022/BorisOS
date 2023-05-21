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
%assign root_dir_size (num_of_root_dir*32)/bytes_per_sector
%assign data_start root_dir+root_dir_size
;;--------------------------------------------------------
;;assign where to put the sectors
%assign search_sector_address 0x7E00
%assign disk_address_packet_struct 0x8000
;;--------------------------------------------------------
;;we assign where the loader will be, needs to be 16 bytes alligned
%assign loader_memory_address 0x500 

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





start:

  
   xor ax, ax
   


    xor ax,ax
    mov ds, ax
    mov ax, 0x1000
    mov ss, ax ; this can grow to 0x10000
    mov sp, 0xFFFF ; this is arbitrary and the stack could be whatever it wants??
    
    
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

        
        
        
        mov word [disk_address_packet_struct+4], (search_sector_address) ;offset of placement
        mov word [disk_address_packet_struct+6], 0 ;segment of placement
        mov dword [disk_address_packet_struct+8] , edx ; this is in sectors
       
            
            ;----------------------We call int 13h-----------------------------
            call load_sector
            ;---------------------------------------------------------
            
        

        mov bx, search_sector_address ;; we reset bx
        add edx, 1 ;;increment dx
        ;----------------------------------------------------------------------------------------
        no_new_sector:
        
        jmp is_loader ;; here we check if its LOADER.SYS


        found_loader: ; congrats we have found the loader
            
            mov ax, word [bx+0x1A] ;; we move the starting cluster into ax
            mov ebx, loader_memory_address
            call load_loader_cluster
                
            ;------------------Call the loader------------------------------
                ;the stack is already set up, we have one os stack
               
                call (loader_memory_address/16):20 ;;TODO make this more flexible
            ;-----------------------------------------------------------------
            jmp $

        no_loader_found_here:
        
        add bx,32

    jmp loop
    end: ; we assume no loader foudnd
    mov ah, 0x0E
	mov bh, 0x00

    ;we shall unroll the code for space reasons
    mov al, 'N'
    int 10h
    mov al, 'L'
    int 10h
    

jmp $


;;=========================Load loader=============================
;; two arguments
;; ax : the starting cluster number
;; ebx : cluster memory offset
load_loader_cluster:
    ;-----------Load the cluster--------------------
        mov ecx, ebx
        and cx , 0x0000000F ; mod 16
        mov edx, ebx
        shr edx, 4 ; /16

       
        
        mov word [disk_address_packet_struct+4], cx ;offset of placement
        mov word [disk_address_packet_struct+6], dx ;dx ;segment of placement
        
        add ax,  (data_start-2) ; this is the formula idk why
        and eax, 0x0000FFFF ; we clear the upper word of eax
        mov dword [disk_address_packet_struct+8] , eax ; this is in sectors ; the cluster which to load 
        sub ax,  (data_start-2)
        
      
        

        call load_sector
        
                

    add ebx , bytes_per_sector ; change the memory offset

    push ebx ; we need to preserve bx
    ;-----------------------------------------------
    ;------------Look up fat for the next one------------
        ;;load the FAT portion which we need

        shl ax, 1 ; multiply by 2 because FAT entry has 2 bytes
        
        
        mov bx, ax ; bx stores the offset of the cluster in the loaded search sector
        and bx, 111111111b ; mod 512

        shr ax, 9 ; divide by bytes_per_sector to get which sector to read for fat
        add ax, FAT_start


        and eax, 0xFFFF
        mov word [disk_address_packet_struct+4], (search_sector_address) ;offset of placement
        mov word [disk_address_packet_struct+6], 0 ;segment of placement
        mov dword [disk_address_packet_struct+8] , eax ; this is in sectors

        call load_sector

    
        ;;now we look inside the search sector
        add bx, search_sector_address


        cmp word [bx], 0xFFF8
        jb .continue_loading
        pop ebx
        ret; we have reached end of file
        .continue_loading:
        mov ax, word [bx]
        
        pop ebx

        call load_loader_cluster
        
;-----------------------------------------------
ret;;TODO
;;=============================================================

;---------------------We check if its the loader directory entry------------
;;we have it in memory at bx
is_loader:
    cmp byte [bx],'L'
    jne no_loader_found_here

    cmp byte [bx+1],'O'
    jne no_loader_found_here

    cmp byte [bx+2],'A'
    jne no_loader_found_here

    cmp byte [bx+3],'D'
    jne no_loader_found_here

    cmp byte [bx+4],'E'
    jne no_loader_found_here

    cmp byte [bx+5],'R'
    jne no_loader_found_here

    cmp byte [bx+6],' '
    jne no_loader_found_here

    cmp byte [bx+7],' '
    jne no_loader_found_here

    cmp byte [bx+8],'S'
    jne no_loader_found_here

    cmp byte [bx+9],'Y'
    jne no_loader_found_here

    cmp byte [bx+10],'S'
    jne no_loader_found_here

    jmp found_loader

;--------------------------------------------------------------------

;---------------------No int 13h extention support message-----------
no_int13_ext:
    mov ah, 0x0E
	mov bh, 0x00

    ;we shall unroll the code for space reasons
    
    mov al, 'E'
	int 10h

jmp $;; TODO Make this say press enter to continue 
;-------------------------------------------------------------

;;========================Load the search sector=============================
;;for whatever it may be
;;arguments shall be passed through memory
;;@ location search_sector_address
;;some params should be specified before calling
;;note the commented code in the procedure
;;we dont need to move the stack because we are not using it

load_sector:
    mov byte [disk_address_packet_struct], 0x10 ;size of packet is 16 bytes
    mov byte [disk_address_packet_struct+1],0 ; always 0
    mov word [disk_address_packet_struct+2],1 ; number of sectors to transfer
    ;mov word [disk_address_packet_struct+4], (search_sector_address) ;offset of placement
    ;mov word [disk_address_packet_struct+6], 0 ;segment of placement
    ;mov dword [disk_address_packet_struct+8] , edx ; this is in sectors
    mov dword [disk_address_packet_struct+12],0 ; should a word or a dword be here?? i have no clue, because its 32 bit i think its word but whatever
    ;--------------------------call int 13h-----------------------
        pusha
        mov dl, 0x80 ;;TODO make this flexible
        xor ax, ax
        mov ds, ax
        mov ah, 0x42
        mov si, disk_address_packet_struct
        int 0x13
        
        popa
    ;----------------------------------------------------------
ret



;;==================================================================================

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

db 0xFF
db 0xFF

times 506 db 0 ;;512 - the four bytes already defined

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

;;=====================DATA START==========================
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