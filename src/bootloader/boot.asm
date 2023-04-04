org 0x7c00 ; Tells the assembler where the program should be
bits 16 ; Tells the assembler to emit 16-bit code

%define ENDL 0x0D, 0x0A

jmp short start
nop

; Headers required by the FAT12 filesystem
oem: db "MSWIN4.1"
sector_bytes: dw 512
sector_clusters: db 1
reserved_sectors: dw 1
fat_count: db 2
dir_entries: dw 0e0h
sectors: dw 2880
descriptor_type: db 0f0h
fat_sectors: dw 9
track_sectors: dw 18
heads: dw 2
hidden_sectors: dd 0
large_sectors: dd 0

; Extended boot records
drive_number: db 0
db 0
signature: db 29h
volume_id: db 12h, 34h, 56h 78h
volume_label: db " PikeOS "
system_id: db " FAT12 "

start:
    jmp main

; Save the register values before they're modified
puts:
    push si
    push ax

.loop:
    lodsb
    or al, al
    jz .done

    mov ah, 0x0e
    mov bh, 0
    int 0x10

    jmp .loop

.done:
    pop ax
    pop si
    ret

main:
    ; Setup data segments
    mov ax, 0
    mov ds, ax
    mov es, ax

    ; Setup stack
    mov ss, ax
    mov sp, 0x7c00

    ; Print message
    mov si, message
    call puts

    hlt

.halt:
    jmp .halt

message: db "test", ENDL, 0

times 510-($-$$) db 0 ; Pad the program with zeros so it creates a 512 byte sector
dw 0aa55h ; BIOS signature