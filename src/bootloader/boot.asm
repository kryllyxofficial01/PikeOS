org 0x7c00 ; The starting location of the operating system
bits 16 ; Tells the assembler to emit 16-bit code

%define endl 0x0d, 0x0a

jmp short main
nop

; BIOS Parameter Block (BPB)
_bpb_oem_identifier: db "MSWIN4.1"
_bpb_bytes_per_sector: dw 512
_bpb_sectors_per_cluster: db 1
_bpb_reserved_sectors: dw 1
_bpb_FAT_count: db 2
_bpb_dir_entries_count: dw 0x0e0
_bpb_total_sectors: dw 2880
_bpb_media_descriptor_type: db 0x0f0
_bpb_sectors_per_FAT: dw 9
_bpb_sectors_per_track: dw 18
_bpb_heads: dw 2
_bpb_hidden_sectors: dd 0
_bpb_large_sectors: dd 0

; Extended Boot Record (EBR)
_ebr_drive_number: db 0
_ebr_reserved: db 0
_ebr_signature: db 0x29
_ebr_volume_ID: db 0x12, 0x34, 0x56, 0x78
_ebr_volume_label: db "PIKEOS     "
_ebr_system_ID: db "FAT12      "

main:
	mov ax, 0

	; Setup data segments
	mov ds, ax
	mov es, ax

	; Setup the stack
	mov ss, ax
	mov sp, 0x7c00

	mov [_ebr_drive_number], dl
	
	mov ax, 1
	mov cl, 1
	mov bx, 0x7e00
	call disk_read

	; Print a string
	mov si, msg
	call puts

	cli
	hlt

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
.loop:
	pusha
	stc

	int 0x13
	jnc .done
	
	popa
	call disk_reset

	; Check if all three retries have expired
	dec di
	test di, di
	jnz .loop

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

; Print a string to the screen
puts:
	; Save the orginal contents of the registers being modified
	push si
	push ax

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
	pop ax
	pop si

	ret

floppy_error:
	mov si, read_failed
	call puts

wait_to_reboot:
	mov ah, 0
	int 0x16
	jmp 0x0ffff:0

.halt:
	cli
	hlt

msg: db "this is a test", endl, 0
read_failed: db "failed to read from floppy", endl, 0

times 510-($-$$) db 0
dw 0xaa55
