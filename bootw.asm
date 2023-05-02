[org 0x7c00]



global start

;;========================Variables for the FAT16 file system==========================
%define jump_point 0x3E
%define OEM_ID 'BSD  4.4' 
%define bytes_per_sector 512
;in code we assume this is always 1
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
%define root_dir_size (num_of_root_dir*32)/512
;;--------------------------------------------------------
;;Define where to put the sectors
%define search_sector_address 0x7E00
%define disk_packet_struct 0x8000
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

    push dx
    call hprint
    xor ax, ax
   
   xor ax, ax             ; make it zero
   mov ds, ax             ; DS=0
   mov ss, ax             ; stack starts at seg 0
   mov sp, 0x9c00         ; 2000h past code start, 
                          ; making the stack 7.5k in size
 
   cli                    ; no interrupts
   push ds                ; save real mode
 
   lgdt [gdtinfo]         ; load gdt register
 
   mov  eax, cr0          ; switch to pmode by
   or al,1                ; set pmode bit
   mov  cr0, eax
   jmp 0x8:pmode
 
pmode:
   mov  bx, 0x10          ; select descriptor 2
   mov  ds, bx            ; 10h = 10000b
 
   and al,0xFE            ; back to realmode
   mov  cr0, eax          ; by toggling bit again
   jmp 0x0:unreal
 
unreal:
   pop ds                 ; get back old segment
   sti
 
   mov bx, 0x0f01         ; attrib/char of smiley
   mov eax, 0x0b8000      ; note 32 bit offset
   mov word [ds:eax], bx

   




 
   jmp $                  ; loop forever
 


gdtinfo:
   dw gdt_end - gdt - 1   ;last byte in table
   dd gdt                 ;start of table
 
gdt:        dd 0,0        ; entry 0 is always unused
codedesc:   db 0xff, 0xff, 0, 0, 0, 10011010b, 00000000b, 0
flatdesc:   db 0xff, 0xff, 0, 0, 0, 10010010b, 11001111b, 0
gdt_end:
 
   times 510-($-$$) db 0  ; fill sector w/ 0's
   dw 0xAA55 

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