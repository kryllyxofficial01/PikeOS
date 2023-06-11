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

	; Print a string
	mov si, msg
	call puts

	hlt

.halt:
	jmp .halt

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

msg: db "this is a test", endl, 0

times 510-($-$$) db 0
dw 0xaa55
