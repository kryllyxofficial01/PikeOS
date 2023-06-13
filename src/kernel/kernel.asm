org 0x0
bits 16

%define endl 0x0d, 0x0a

main:
	; Print a string
	mov si, msg
	call puts

.halt:
	cli
	hlt

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
