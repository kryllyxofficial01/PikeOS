bits 16

section _TEXT class=CODE

global _write_char
_write_char:
    push bp
    mov bp, sp
    push bx

    mov ah, 0x0e
    mov al, [bp+4]
    mov bh, [bp+6]

    int 0x10

    pop bx
    mov sp, bp
    pop bp

    ret