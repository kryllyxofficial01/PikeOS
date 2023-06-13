org 0x7c00 ; The starting location of the operating system
bits 16 ; Tells the assembler to emit 16-bit code

%define endl 0x0d, 0x0a

jmp short start
nop

; BIOS Parameter Block (BPB)
_bpb_oem_identifier: db "MSWIN4.1"
_bpb_bytes_per_sector: dw 512
_bpb_sectors_per_cluster: db 1
_bpb_reserved_sectors: dw 1
_bpb_FAT_count: db 2
_bpb_dir_entry_count: dw 0x0e0
_bpb_total_sectors: dw 2880
_bpb_media_descriptor_type: db 0x0f0
_bpb_sectors_per_FAT: dw 9
_bpb_sectors_per_track: dw 18
_bpb_heads: dw 2
_bpb_hidden_sectors: dd 0
_bpb_large_sectors: dd 0

; Extended Boot Record (EBR)
_ebr_drive_number: db 0
db 0 ; reserved byte
_ebr_signature: db 0x29
_ebr_volume_ID: db 0x12, 0x34, 0x56, 0x78
_ebr_volume_label: db "PIKEOS     "
_ebr_system_ID: db "FAT12      "

start:
	mov ax, 0

	; Setup data segments
	mov ds, ax
	mov es, ax

	; Setup the stack
	mov ss, ax
	mov sp, 0x7c00

	push es
	push word .after
	retf

.after:
	mov [_ebr_drive_number], dl
	
	; Print loading message
	mov si, loading
	call puts

	push es
	mov ah, 0x08
	int 0x13
	jc floppy_error
	pop es

	and cl, 0x3f
	xor ch, ch
	mov [_bpb_sectors_per_track], cx

	inc dh
	mov [_bpb_heads], dh

	; Calculate the LBA
	mov ax, [_bpb_sectors_per_FAT]
	mov bl, [_bpb_FAT_count]
	xor bh, bh
	mul bx
	add ax, [_bpb_reserved_sectors]
	push ax

	; Calculate the size of the root directory
	mov ax, [_bpb_dir_entry_count]
	shl ax, 5
	xor dx, dx
	div word [_bpb_bytes_per_sector]

	test dx, dx
	jz .root_dir_after
	inc ax

.root_dir_after:
	; Read from the root directory
	mov cl, al
	pop ax
	mov dl, [_ebr_drive_number]
	mov bx, buffer
	call disk_read

	xor bx, bx
	mov di, buffer

.search_for_kernel:
	; Look for the kernel
	mov si, kernel
	mov cx, 11
	push di
	repe cmpsb
	pop di
	je .found_kernel

	add di, 32
	inc bx
	cmp bx, [_bpb_dir_entry_count]
	jl .search_for_kernel

	jmp kernel_not_found_error

.found_kernel:
	mov ax, [di+26]
	mov [kernel_cluster], ax

	; Load FAT from memory
	mov ax, [_bpb_reserved_sectors]
	mov bx, buffer
	mov cl, [_bpb_sectors_per_FAT]
	mov dl, [_ebr_drive_number]
	call disk_read

	; Read the kernel
	mov bx, KERNEL_SEGMENT
	mov es, bx
	mov bx, KERNEL_OFFSET

.load_kernel_loop:
	mov ax, [kernel_cluster]
	add ax, 31 ; Only works on a 1.44mb floppy

	mov cl, 1
	mov dl, [_ebr_drive_number]
	call disk_read

	; Calculate the next cluster
	add bx, [_bpb_bytes_per_sector]
	mov ax, [kernel_cluster]
	mov cx, 3
	mul cx
	mov cx, 2
	div cx

	mov si, buffer
	add si, ax
	mov ax, [ds:si]

	or dx, dx
	jz .even

.odd:
	shr ax, 4
	jmp .next_cluster

.even:
	and ax, 0x0fff

.next_cluster:
	cmp ax, 0x0ff8
	jae .read_finished

	mov [kernel_cluster], ax
	jmp .load_kernel_loop

.read_finished:
	; Jump to the kernel
	mov dl, [_ebr_drive_number]
	mov ax, KERNEL_SEGMENT
	mov ds, ax
	mov es, ax

	jmp KERNEL_SEGMENT:KERNEL_OFFSET

	jmp wait_to_reboot ; if this ever runs, you should as well

	cli
	hlt

floppy_error:
	mov si, read_failed
	call puts
	jmp wait_to_reboot

kernel_not_found_error:
	mov si, kernel_not_found
	call puts
	jmp wait_to_reboot

wait_to_reboot:
	mov ah, 0
	int 0x16
	jmp 0x0ffff:0

.halt:
	cli
	hlt

; Print a string to the screen
puts:
	; Save the orginal contents of the registers being modified
	push si
	push ax
	push bx

.loop:
	lodsb ; Load the next character

	; Check if the current character is NULL
	or al, al
	jz .done

	; Print the character
	mov ah, 0x0e
	mov bh, 0
	int 0x10
	
	jmp .loop

.done:
	; Restore the modified registers
	pop bx
	pop ax
	pop si

	ret

; Convert an LBA address to a CHS address
lba_to_chs:
	push ax
	push dx

	; Get the sector
	xor dx, dx
	div word [_bpb_sectors_per_track]
	inc dx
	mov cx, dx

	; Get the cylinder and head
	xor dx, dx
	div word [_bpb_heads]

	; Move results into respective output registers
	mov dh, dl
	mov ch, al
	shl ah, 6
	or cl, ah

	pop ax
	mov dl, al
	pop ax

	ret

; Read from the disk
disk_read:
	push ax
	push bx
	push cx
	push dx
	push di

	push cx
	call lba_to_chs
	pop ax

	mov ah, 0x02
	mov di, 3

; Try the read operation three times
.retry:
	pusha
	stc

	int 0x13
	jnc .done
	
	popa
	call disk_reset

	; Check if retries have expired
	dec di
	test di, di
	jnz .retry

.fail:
	jmp floppy_error

.done:
	popa

	pop di
	pop dx
	pop cx
	pop bx
	pop ax

	ret

disk_reset:
	pusha

	mov ah, 0
	stc
	int 0x13

	jc floppy_error

	popa
	ret

loading: db "Loading...", endl, 0

read_failed: db "Failed to read from floppy", endl, 0
kernel_not_found: db "Couldn't find kernel", endl, 0

kernel: db "KERNEL  BIN"

kernel_cluster: dw 0
KERNEL_SEGMENT equ 0x2000
KERNEL_OFFSET equ 0

times 510-($-$$) db 0
dw 0xaa55

buffer:
