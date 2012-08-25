; %--------------------%
; | PROGRAM: kernel loader (어셈블리 부분)
; | SUMMARY:	별로 하는일 없다..
; |				1. GDT 세팅해서 프로텍티드 모드 진입하고
; |				2. c 로더로 점프한다.

; %--------------------%
; | ROUTINE: 16비트 커널 로더 코드.
; | SUMMARY: GDT를 세팅하여 32비트 모드로 전환 후, 32비트 코드로 넘어간다.
[org 0]
[bits 16]
	mov ax, cs
	mov ds, ax

	mov ax, 0x2000
	mov ss, ax
	mov sp, 0x0
	mov bp, 0x0

	cli

; %---------------------%
; | ROUTINE: copy GDTR and GDT to appropriate position (0x0, 0x6)
	mov ax, 0
	mov es, ax
	mov si, gdtr
	mov di, 0
	mov cx, 6
	rep movsb

	mov ax, 0
	mov es, ax
	mov si, gdt
	mov di, 0x6
	mov cx, gdt_end - gdt - 1
	rep movsb

; %-------------------%
; | ROUTINE: load GDTR to register and switch to protected mode
	lgdt [gdtr]
	
	mov eax, cr0
	or eax, 0x00000001
	mov cr0, eax

; %--------------------%
; | ROUTINE: jump to 32 bit kernel code
	jmp dword KernCodeSelector:Code32+0x20000
	nop
	nop

; %--------------------%
; | DATA STRUCTURE: GDTR and GDT table
GDT_SEGMENT	equ 0
GDT_OFFSET	equ	0

gdtr:
	dw gdt_end - gdt
	dd 0x6

gdt:
	dd 0x0, 0x0
KernCodeSelector	equ 0x08
	dw 0xFFFF, 0x0000
	db 0x00, 0x9A, 0xCF, 0x00
KernDataSelector	equ	0x10
	dw 0xFFFF, 0x0000
	db 0x00, 0x92, 0xCF, 0x00
UserCodeSelector	equ 0x18+3
	dw 0xFFFF, 0x0000
	db 0x00, 0xFA, 0xCF, 0x00
UserDataSelector	equ 0x20+3
	dw 0xFFFF, 0x0000
	db 0x00, 0xF2, 0xCF, 0x00
gdt_end:

; %---------------------%
; | ROUTINE: 32비트 커널 로더 코드. 시작 위치가 0이 아님에 주의.
; |			 C로 된 커널 로더로 점프함.
[bits 32]

Code32:
	mov ax, KernDataSelector
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	
	mov esp, 0x20000
	mov ebp, esp

	jmp KernCodeSelector:0x20200

times 512-($-$$) db 0
