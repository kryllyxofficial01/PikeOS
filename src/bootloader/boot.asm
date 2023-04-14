org 0x7c00 ; Tells the assembler where to load the code in memory
bits 16 ; Tells the assembler to emit 16-bit code

; Newline character
%define ENDL 0x0d, 0x0a

jmp short start
nop

; FAT Headers
oem: db 'MSWIN4.1'
bytes_per_sector: dw 512
sectors_per_cluster: db 1
reserved_sectors: dw 1
FAT_count: db 2
dir_entries: dw 0x0e0
sectors: dw 2880
media_descriptor_type: db 0x0f0
sectors_per_FAT: dw 9
sectors_per_track: dw 18
heads: dw 2
hidden_sectors: dd 0
large_sectors: dd 0

; Extended Boot Record
drive_number: db 0
db 0 ; Reserved byte
signature: db 0x29
volumnID: db 0x78, 0x56, 0x34, 0x12
volumn_label: db "PIKE     OS" ; Contents are irrelevant, just needs to be 11 bytes
systemID: db "FAT12   "

start:
    jmp main

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

main:
    mov ax, 0

    ; Setup data segments
    mov ds, ax
    mov es, ax

    ; Setup stack
    mov ss, ax
    mov sp, 0x7c00 ; Offset the stack to start before the code segment

    ; Read something from the disk
    mov [drive_number], dl
    mov ax, 1
    mov cl, 1
    mov bx, 0x7e00
    call disk_read

    ; Print a string
    mov si, msg
    call print_str

    cli
    hlt

; Disk read failed
floppy_error:
    mov si, read_failed
    call print_str
    jmp wait_to_reboot

; Reboot the OS upon keypress
wait_to_reboot:
    mov ah, 0
    int 0x16
    jmp 0x0FFFF:0 ; Jump to the beginning of the BIOS

.halt:
    cli
    hlt

; Convert an LBA address to a CHS address
lba_to_chs:
    ; Save the original values of the registers modified collaterally
    push ax
    push dx

    ; Calculate the sector
    xor dx, dx
    div word [sectors_per_track]
    inc dx
    mov cx, dx

    ; Calculate the head and cylinder
    xor dx, dx
    div word [heads]

    ; Store the results in the registers the BIOS expects them to be in
    mov dh, dl
    mov ch, al
    shl ah, 6
    or cl, ah

    pop ax
    mov dl, al
    pop ax

    ret

; Read from the disk
disk_read:
    ; Save the original values of the to-be modified registers
    push ax
    push bx
    push cx
    push dx
    push di

    push cx
    call lba_to_chs
    pop ax

    mov ah, 0x02
    mov di, 3 ; Try read operation 3 times

; Retry reading from disk if failure
.retry:
    pusha
    stc

    int 0x13
    jnc .done

    popa
    call disk_reset

    dec di
    test di, di
    jnz .retry

; All read attempts failed
.failed:
    jmp floppy_error

.done:
    popa

    pop di
    pop dx
    pop cx
    pop bx
    pop ax

    ret

; Reset the disk controller
disk_reset:
    pusha

    mov ah, 0
    stc
    int 0x13

    jc floppy_error

    popa
    ret

; Variables
msg: db 'Hello, world!', ENDL, 0
read_failed: db 'Failed to read from disk...', ENDL, 0

; BIOS stuff
times 510-($-$$) db 0 ; Pad the program to create a 512 byte sector
dw 0x0aa55 ; Signature that tells the BIOS this is an OS