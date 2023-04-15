org 0x0
bits 16

; Newline character
%define ENDL 0x0d, 0x0a

start:
    mov si, msg
    call print_str

.halt:
    cli
    hlt

; Save the values of the to-be modified registers
print_str:
    push si
    push ax
    push bx

; Print a string to the screen
.loop:
    lodsb ; Load the next character

    ; Check if current character is NULL
    or al, al
    jz .done

    ; Print the character
    mov ah, 0x0e
    mov bh, 0
    int 0x10

    jmp .loop

.done:
    ; Restore registers
    pop bx
    pop ax
    pop si

    ret

msg: db 'Hello, world!', ENDL, 0