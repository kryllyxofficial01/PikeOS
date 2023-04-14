org 0x7c00 ; Tells the assembler where to load the code in memory
bits 16 ; Tells the assembler to emit 16-bit code

; Newline character
%define ENDL 0x0d, 0x0a

start:
    jmp main

; Print a string to the screen
print_str:
    ; Save the values of the to-be modified registers
    push si
    push ax

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
    pop ax
    pop si

    ret

main:
    mov ax, 0

    ; Setup data segments
    mov ds, ax
    mov es, ax

    ; Setup stack
    mov ss, ax
    mov sp, 0x7c00 ; Offset the stack to start before the code segment

    ; Print a string
    mov si, msg
    call print_str

    hlt

.halt:
    jmp .halt

; Variables
msg: db 'Hello, world!', ENDL, 0

; BIOS stuff
times 510-($-$$) db 0 ; Pad the program to create a 512 byte sector
dw 055aah ; Signature that tells the BIOS this is an OS