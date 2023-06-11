org 0x7c00 ; The starting location of the operating system
bits 16 ; Tells the assembler to emit 16-bit code

%define endl 0x0d, 0x0a

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
