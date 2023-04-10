org 0x0
bits 16

%define ENDL 0x0D, 0x0A

start:
    mov si, msg
    call puts

.halt:
    cli
    hlt

puts:
    push si
    push ax
    push bx

.loop:
    lodsb
    or al, al
    jz .done

    mov ah, 0x0e
    mov bh, 0
    int 0x10

    jmp .loop

.done:
    pop bx
    pop ax
    pop si

    ret

msg: db 'test', ENDL, 0