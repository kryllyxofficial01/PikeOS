bits 16

section _TEXT class=CODE

global _x86_write_char
_x86_write_char:
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

global _x86_div
_x86_div:
    push bp
    mov bp, sp
    push bx

    mov eax, [bp+8]
    mov ecx, [bp+12]
    xor edx, edx
    div ecx

    mov bx, [bp+16]
    mov [bx+4], eax

    mov eax, [bp+4]
    div ecx

    mov [bx], eax
    mov bx, [bp+18]
    mov [bx], edx

    pop bx
    mov sp, bp
    pop bp

    ret