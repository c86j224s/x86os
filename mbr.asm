; %--------------------%
; | PROGRAM: boot sector (MBR)
; | SUMMARY:	CPU가 처음 로드한 MBR의 0x3E 번지부터의 부트코드이다.
; |				FAT16 파티션의 FAT과 ROOT DIR ENTRY를 읽어와서
; |				최상위 BOOT 파일을 찾아 해당 파일을 메모리에 로드,
; |				해당 위치로 점프하는 기능을 하고 있다.
; |				BOOT 파일은 메모리의 0x20000에 로드된다.
; |				부팅 과정에서 실패에 대해서는 예외처리를 하지 않았다.
; |				코드 길이가 아슬아슬하더군 -_-;;;

; %--------------------%
; | CONSTANTS: MBR의 FAT 테이블 관련한 정보들
BYTES_PER_SECTOR			equ 11	; word
SECTORS_PER_CLUSTER			equ 13	; byte
RESERVED_SECTOR_COUNT		equ 14	; word
NUMBER_OF_FATS				equ 16	; byte
ROOT_DIRECTORY_ENTRY_COUNT	equ 17	; word
FAT_SIZE					equ 22	; word
SECTORS_PER_TRACK			equ 24	; word
NUMBER_OF_HEAD				equ 26	; word

; %--------------------%
; | CONSTANTS: 전역 변수 선언 (주소 할당)
DATA_AREA_ADDRESS			equ 0x200
starting_cluster			equ	0x202

; %--------------------%
; | CONSTANTS: FAT 테이블과 루트 디렉토리 엔트리 테이블이 올라갈 위치
FAT_ADDR_SEG			equ 0x1000
FAT_ADDR_OFFSET			equ 0

; %--------------------%
; | CONSTANTS: 루트 디렉토리 엔트리 테이블이 올라갈 위치
ROOT_DIR_ENT_ADDR_SEG		equ 0x2000
ROOT_DIR_ENT_ADDR_OFFSET	equ 0

; %--------------------%
; | CONSTANTS: 커널 이미지가 올라갈 위치
KERNEL_ADDR_SEG			equ 0x2000
KERNEL_ADDR_OFFSET		equ 0

; %--------------------%
; | ROUTINE:	코드, 데이터 세그먼트를 설정하고 스택을 세팅함.
; |				세팅한 스택의 위치는 0x8c00임.
[org 0x3e]
[bits 16]
	jmp 0x07C0:start
start:
	mov ax, cs
	mov ds, ax

	mov ax, 0x7c0
	mov ss, ax
	mov ax, 0x1000
	mov bp, ax
	mov sp, ax
	
; %--------------------%
; | ROUTINE:	FAT 테이블을 지정된 위치로 로드
; |				FAT 테이블은 RESERVED_SECTOR_COUNT번 섹터에 위치해 있고,
; |				사이즈는 FAT_SIZE섹터이다. 
; |				FAT_SIZE가 255섹터 이상일 경우는 문제가 생길 것 같다.
read_fat_table:
	mov ax, word [RESERVED_SECTOR_COUNT]
	mov dword [lba1], eax
	mov word [transfer_buffer_segment], FAT_ADDR_SEG
	mov word [transfer_buffer_offset], FAT_ADDR_OFFSET
	mov ax, word [FAT_SIZE]
	mov byte [number_of_sectors], al
	call read_sector
	
	mov ax, 0xB800
	mov es, ax
	mov byte [es:0], 'B'
	mov byte [es:1], 0x06

; %--------------------%
; | ROUTINE:	루트 디렉토리 엔트리 테이블을 로드. 
; |				루트 디렉토리 엔트리 테이블의 위치는,
; |				(RESERVED_SECTOR_COUNT*NUMBER_OF_FATS*FAT_SIZE)섹터.
; |				그리고 사이즈는 ROOT_DIRECTORY_ENTRY_COUNT섹터이다.
; |				FAT 로드처럼, 사이즈가 255섹터를 넘으면 문제가 생길 듯 하다.
read_directory_entry_table:
	mov ax, word [RESERVED_SECTOR_COUNT]
	mov ch, 0
	mov cl, byte [NUMBER_OF_FATS]
	loop_add_fat__:
		add ax, word [FAT_SIZE]
		dec cx
		jnz loop_add_fat__
	mov dword [lba1], eax
	mov word [transfer_buffer_segment], ROOT_DIR_ENT_ADDR_SEG
	mov word [transfer_buffer_offset], ROOT_DIR_ENT_ADDR_OFFSET
	mov ax, word [ROOT_DIRECTORY_ENTRY_COUNT]
	shr ax, 4
	mov byte [number_of_sectors], al
	call read_sector
	
	mov ax, 0xB800
	mov es, ax
	mov byte [es:2], 'O'
	mov byte [es:3], 0x06

; %--------------------%
; | ROUTINE:	부트파일 검색
; |				"boot"라는 이름을 가지는 파일을 루트 디렉토리에서 검색하여
; |				그 파일의 내용을 담고 있는 시작 클러스터를 찾아낸다.
find_boot:
	call find_cluster
	mov word [starting_cluster], ax

	mov ax, 0xB800
	mov es, ax
	mov byte [es:4], 'O'
	mov byte [es:5], 0x06

; %--------------------%
; | ROUTINE:	부트 파일의 내용 클러스터를 읽어 메모리에 적재한다.
read_boot:
	call read_cluster

	mov ax, 0xB800
	mov es, ax
	mov byte [es:6], 'T'
	mov byte [es:7], 0x06

; %--------------------%
; | ROUTINE:	부트 이미지로의 점프 (0x20000)
jump_boot:
	jmp KERNEL_ADDR_SEG:KERNEL_ADDR_OFFSET

; %--------------------%
; | GLOBAL VARIABLE:	INT13H에 쓰이는 디스크 어드레스 패킷 구조체
disk_address_packet:
	size_of_packet					db 0x10
	unused1							db 0x0
	number_of_sectors				db 1
	unused2							db 0x0
	transfer_buffer_offset			dw 0
	transfer_buffer_segment			dw 0
	lba1							dd 1
	lba2							dd 0
; %--------------------%
; | GLOBAL VARIABLE:	파티션에 저장된 부트 파일의 이름 (11byte)
entry_name db 'B', 'O', 'O', 'T', 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20

; %--------------------%
; | FUNCTION: read_sector
; | PARAMETERS:	NONE (prepare the Global Data Address Packet)
; | EXAMPLE: SEE BELOW
; |				mov dword [lba2], 0
; |				mov word [transfer_buffer], 0
; |				mov word [transfer_buffer+2], 0
; |				mov byte [number_of_sectors], 1
; |				call read_sector
; |				add sp, 8
read_sector:
	mov ah, 0x42
	mov dl, 0x80
	mov si, disk_address_packet
	int 0x13
	ret

; %--------------------%
; | FUNCTION: strncmp
; | PARAMETER:	REGISTER(	es = segment of string2
; |							bp+6 = offset of string2
; |						)
; | RETURN VALUE:	ax (같으면 0, 다르면 1)
strncmp:
	mov cx, 11
	mov bx, entry_name

	cmp_char__:
		mov al, byte [bx]
		mov ah, byte [es:di]
		xor ah, al
		jnz cmp_fail__
		inc bx
		inc di
		dec cx
		jz cmp_success__
		jmp cmp_char__
	
	cmp_fail__:
		mov ax, 1
		jmp cmp_end__
	
	cmp_success__:
		mov ax, 0

	cmp_end__:
		ret

; %-------------------%
; | FUNCTION: find_cluster
; | PARAMETER: NONE
; | RETURN VALUE:	eax (성공시 유의미한 값, 실패시 0)
find_cluster:
	mov ax, ROOT_DIR_ENT_ADDR_SEG
	mov es, ax
	mov si, ROOT_DIR_ENT_ADDR_OFFSET

	cmp_entry__:
		mov al, byte [es:si]
		
		xor al, 0xE5
		jz next_entry__
		xor al, 0x0
		jz find_fail__

		mov di, si
		call strncmp
		or al, al
		jnz next_entry__

		mov eax, 0
		mov al, byte [es:si+21]
		shl eax, 8
		mov al, byte [es:si+20]
		shl eax, 16
		mov ah, byte [es:si+27]
		mov al, byte [es:si+26]
		jmp find_success__

	next_entry__:
		add si, 32
		jmp cmp_entry__
	
	find_fail__:
		mov eax, 0

	find_success__:
		ret

; %--------------------%
; | FUNCTION: read_cluster
; | PARAMETER: NONE
read_cluster:
	mov ax, word [RESERVED_SECTOR_COUNT]
	mov ch, 0
	mov cl, byte [NUMBER_OF_FATS]
	rc0000__:
		add ax, word [FAT_SIZE]
		dec cx
		jnz rc0000__
	mov bx, word [ROOT_DIRECTORY_ENTRY_COUNT]
	shr bx, 4
	add ax, bx
	mov word [DATA_AREA_ADDRESS], ax

	mov word [transfer_buffer_segment], KERNEL_ADDR_SEG
	mov word [transfer_buffer_offset], KERNEL_ADDR_OFFSET

	mov al, byte [SECTORS_PER_CLUSTER]
	mov byte [number_of_sectors], al

	rc0001__:
		mov eax, 0
		mov ax, word [starting_cluster]
		sub ax, 2
		mov bh, 0
		mov bl, byte [SECTORS_PER_CLUSTER]
		mul bx
		add ax, word [DATA_AREA_ADDRESS]
		mov dword [lba1], eax
		call read_sector

		mov ax, FAT_ADDR_SEG
		mov fs, ax
		mov di, FAT_ADDR_OFFSET
		shl word [starting_cluster], 1
		add di, word [starting_cluster]
		mov ax, word [fs:di]
		mov word [starting_cluster], ax

		mov ah, 0
		mov al, byte [SECTORS_PER_CLUSTER]
		shl ax, 9
		add word [transfer_buffer_offset], ax

		jnc rc0002__
		add word [transfer_buffer_segment], 1

	rc0002__:
		mov ax, word [starting_cluster]
		xor ax, 0xffff
		jnz rc0001__
	
	ret
