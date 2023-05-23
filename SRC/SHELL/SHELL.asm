bits 16

; glb size_t : unsigned
; glb ptrdiff_t : int
; glb wchar_t : unsigned short
; glb intptr_t : int
; glb uintptr_t : unsigned
; glb intmax_t : int
; glb uintmax_t : unsigned
; glb int8_t : signed char
; glb int_least8_t : signed char
; glb int_fast8_t : signed char
; glb uint8_t : unsigned char
; glb uint_least8_t : unsigned char
; glb uint_fast8_t : unsigned char
; glb int16_t : short
; glb int_least16_t : short
; glb int_fast16_t : short
; glb uint16_t : unsigned short
; glb uint_least16_t : unsigned short
; glb uint_fast16_t : unsigned short
; glb int32_t : int
; glb int_least32_t : int
; glb int_fast32_t : int
; glb uint32_t : unsigned
; glb uint_least32_t : unsigned
; glb uint_fast32_t : unsigned
; glb specify_video_mode : () void
section .text
	global	_specify_video_mode
_specify_video_mode:
	push	ebp
	movzx	ebp, sp
	;sub	sp,          0
L1:
	db	0x66
	leave
	retf
L3:

section .fxnsz noalloc
	dd	L3 - _specify_video_mode

; glb set_cursor_size : () void
section .text
	global	_set_cursor_size
_set_cursor_size:
	push	ebp
	movzx	ebp, sp
	;sub	sp,          0
pushad 
mov ah, 1 
mov ch, 0b00000001 
mov cl, 0x00001110 
int 0x10 

L4:
	db	0x66
	leave
	retf
L6:

section .fxnsz
	dd	L6 - _set_cursor_size

; glb set_cursor_position : (
; prm     row : unsigned char
; prm     column : unsigned char
; prm     page_number : unsigned char
;     ) void
section .text
	global	_set_cursor_position
_set_cursor_position:
	push	ebp
	movzx	ebp, sp
	;sub	sp,          0
; loc     row : (@8) : unsigned char
; loc     column : (@12) : unsigned char
; loc     page_number : (@16) : unsigned char
; if
; RPN'ized expression: "page_number 7 > "
; Expanded expression: "(@16) *(1) 7 > "
; Fused expression:    "> *(@16) 7 IF! "
	mov	al, [bp+16]
	movzx	eax, al
	cmp	eax, 7
	jle	L9
; {
; return
	jmp	L7
; }
L9:
pushad 
mov ah, 0x02 
mov bh, byte [bp+16]
mov dh, byte [bp+8] 
mov dl, byte [bp+4] 
int 0x10 

L7:
	db	0x66
	leave
	retf
L11:

section .fxnsz
	dd	L11 - _set_cursor_position



; Syntax/declaration table/stack:
; Bytes used: 395/15360


; Macro table:
; Macro __SMALLER_C__ = `0x0100`
; Macro __SMALLER_C_32__ = ``
; Macro __UNREAL__ = ``
; Macro __SMALLER_C_SCHAR__ = ``
; Macro __SMALLER_C_UWCHAR__ = ``
; Macro __SMALLER_C_WCHAR16__ = ``
; Bytes used: 123/5120


; Identifier table:
; Ident 
; Ident __floatsisf
; Ident __floatunsisf
; Ident __fixsfsi
; Ident __fixunssfsi
; Ident __addsf3
; Ident __subsf3
; Ident __negsf2
; Ident __mulsf3
; Ident __divsf3
; Ident __lesf2
; Ident __gesf2
; Ident size_t
; Ident ptrdiff_t
; Ident wchar_t
; Ident intptr_t
; Ident uintptr_t
; Ident intmax_t
; Ident uintmax_t
; Ident int8_t
; Ident int_least8_t
; Ident int_fast8_t
; Ident uint8_t
; Ident uint_least8_t
; Ident uint_fast8_t
; Ident int16_t
; Ident int_least16_t
; Ident int_fast16_t
; Ident uint16_t
; Ident uint_least16_t
; Ident uint_fast16_t
; Ident int32_t
; Ident int_least32_t
; Ident int_fast32_t
; Ident uint32_t
; Ident uint_least32_t
; Ident uint_fast32_t
; Ident specify_video_mode
; Ident set_cursor_size
; Ident set_cursor_position
; Ident row
; Ident column
; Ident page_number
; Bytes used: 508/5632

; Next label number: 12
; Compilation succeeded.
