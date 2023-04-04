org 0x7c00 ; Tells the assembler where the program should be
bits 16 ; Tells the assembler to emit 16-bit code

%define ENDL 0x0D, 0x0A

jmp short start
nop

; Headers required by the FAT12 filesystem
oem: db "MSWIN4.1"
sector_bytes: dw 512
cluster_sectors: db 1
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

main:
    mov ax, 0
    mov ds, ax
    mov es, ax

    mov ss, ax
    mov sp, 0x7c00

    mov [drive_number], dl
    mov ax, 1
    mov cl, 1
    mov bx, 0x7e00
    call read_from_disk

    mov si, message
    call puts

    cli
    hlt

disk_read_failed:
    mov si, floppy_error
    call puts
    jmp wait_to_reboot

wait_to_reboot:
    mov ah, 0
    int 16h
    jmp 0ffffh:0

.halt:
    cli
    hlt

lba_to_chs:
    push ax
    push dx

    xor dx, dx
    div word [track_sectors]

    inc dx
    mov cx, dx

    xor dx, dx
    div word [heads]

    mov dh, al
    mov ch, al
    shl ah, 6
    or cl, ah

    pop ax
    mov dl, al
    pop ax

    ret

read_from_disk:
    push ax
    push bx
    push cx
    push dx
    push di

    push cx
    call lba_to_chs
    pop ax

    mov ah, 02h
    mov di, 3

.retry:
    pusha
    stc
    int 13h
    jnc .done

    popa
    call reset_disk

    dec di
    test di, di
    jnz .retry

.fail:
    jmp disk_read_failed

.done:
    popa

    pop di
    pop dx
    pop cx
    pop bx
    pop ax

    ret

reset_disk:
    pusha
    mov ah, 0
    stc
    int 13h
    jc disk_read_failed
    popa

    ret

message: db "test", ENDL, 0
floppy_error: db "Failed to read from disk", ENDL, 0

times 510-($-$$) db 0 ; Pad the program with zeros so it creates a 512 byte sector
dw 0x0aa55 ; BIOS signature