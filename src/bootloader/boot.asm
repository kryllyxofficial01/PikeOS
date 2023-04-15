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
volumeID: db 0x78, 0x56, 0x34, 0x12
volume_label: db "PIKE     OS" ; Contents are irrelevant, just needs to be 11 bytes
systemID: db "FAT12   "

start:
    mov ax, 0

    ; Setup data segments
    mov ds, ax
    mov es, ax

    ; Setup stack
    mov ss, ax
    mov sp, 0x7c00 ; Offset the stack to start before the code segment

    push es
    push word .after
    retf

.after:
    mov [drive_number], dl

    mov si, loading
    call print_str

    push es
    mov ah, 0x08

    int 0x13

    jc floppy_error
    pop es

    and cl, 0x3f
    xor ch, ch
    mov [sectors_per_track], cx

    inc dh
    mov [heads], dh

    mov ax, [sectors_per_FAT]
    mov bl, [FAT_count]
    xor bh, bh
    mul bx
    add ax, [reserved_sectors]
    push ax

    mov ax, [sectors_per_FAT]
    shl ax, 5
    xor dx, dx
    div word [bytes_per_sector]

    test dx, dx
    jz .root_dir
    inc ax

.root_dir:
    mov cl, al
    pop ax
    mov dl, [drive_number]
    mov bx, buffer
    call disk_read

    xor bx, bx
    mov di, buffer

.search_for_kernel:
    mov si, kernel_file
    mov cx, 11 ; 11 is the size in bytes of the file name
    push di

    ; This looks like I fell asleep on my keyboard. I assure you, this is not where I fell asleep...
    repe cmpsb

    pop di
    je .kernel_found

    add di, 32
    inc bx
    cmp bx, [dir_entries]
    jl .search_for_kernel

    jmp kernel_not_found_error

.kernel_found:
    mov ax, [di+26] ; 26 is the offset of the lower 16 bits of the cluster
    mov [kernel_cluster], ax

    mov ax, [reserved_sectors]
    mov bx, buffer
    mov cl, [sectors_per_FAT]
    mov dl, [drive_number]
    call disk_read

    mov bx, KERNEL_SEGMENT
    mov es, bx
    mov bx, KERNEL_OFFSET

.load_kernel_loop:
    mov ax, [kernel_cluster]
    add ax, 31 ; TODO: FIX THIS HARDCODED OFFSET, IT ONLY WORKS FOR A 1MB FLOPPY

    mov cl, 1
    mov dl, [drive_number]
    call disk_read

    add bx, [bytes_per_sector]
    mov ax, [kernel_cluster]
    mov cx, 3
    mul cx
    mov cx, 2
    div cx

    mov si, buffer
    add si, ax
    mov ax, [ds:si]

    or dx, dx
    jz .even

.odd:
    shr ax, 4
    jmp .next_cluster

.even:
    and ax, 0x0fff

.next_cluster:
    cmp ax, 0x0ff8
    jae .read_finished

    mov [kernel_cluster], ax
    jmp .load_kernel_loop

.read_finished:
    mov dl, [drive_number]
    mov ax, KERNEL_SEGMENT
    mov ds, ax
    mov es, ax

    jmp KERNEL_SEGMENT:KERNEL_OFFSET

    jmp wait_to_reboot ; If this ever gets called, run for your life

    cli
    hlt

; Disk read failed
floppy_error:
    mov si, read_failed
    call print_str
    jmp wait_to_reboot

; Couldn't find the kernel
kernel_not_found_error:
    mov si, kernel_not_found
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

loading: db 'Loading...', ENDL, 0
read_failed: db 'Failed to read from disk...', ENDL, 0
kernel_not_found: db 'Could not find kernel', ENDL, 0

kernel_file: db 'KERNEL  BIN'
kernel_cluster: dw 0

KERNEL_SEGMENT equ 0x2000
KERNEL_OFFSET equ 0

; BIOS stuff
times 510-($-$$) db 0 ; Pad the program to create a 512 byte sector
dw 0x0aa55 ; Signature that tells the BIOS this is an OS

buffer: