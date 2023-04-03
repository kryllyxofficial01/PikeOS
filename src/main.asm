org 0x7c00 ; Tells the assembler where the program should be
bits 16 ; Tells the assembler to emit 16-bit code

main:
    hlt

.halt:
    jmp .halt

times 510-($-$$) db 0 ; Pad the program with zeros so it creates a 512 byte sector
dw 0aa55h ; BIOS signature