org 0x7c00
bits 16

%define ENDL 0x0d, 0x0a

jmp short start
nop

oem: db 'MSWIN4.1'
bytes_per_sector: dw 512
sectors_per_cluster: db 1
reserved_sectors: dw 1
fat_count: db 2
dir_entries: dw 0e0h
total_sectors: dw 2880
media_descriptor_type: db 0f0h
sectors_per_fat: dw 9
sectors_per_track: dw 18
heads: dw 2
hidden_sectors: dd 0
large_sectors: dd 0

drive_number: db 0
db 0
signature: db 29h
volume_id: db 12h, 34h, 56h, 78h
volume_label: db 'PIKE    OS'
system_id: db 'FAT12   '

start:
    mov ax, 0
    mov ds, ax
    mov es, ax

    mov ss, ax
    mov sp, 0x7c00

    push es
    push word .after
    retf

.after:
    mov [drive_number], dl

    mov si, loading
    call puts

    push es
    mov ah, 08h
    int 13h
    jc floppy_error
    pop es

    and cl, 0x3f
    xor ch, ch
    mov [sectors_per_track], cx

    inc dh
    mov [heads], dh

    mov ax, [sectors_per_fat]
    mov bl, [fat_count]
    xor bh, bh
    mul bx
    add ax, [reserved_sectors]
    push ax

    mov ax, [dir_entries]
    shl ax, 5
    xor dx, dx
    div word [bytes_per_sector]

    test dx, dx
    jz .root_dir_after
    inc ax

.root_dir_after:
    mov cl, al
    pop ax
    mov dl, [drive_number]
    mov bx, buffer
    call disk_read

    xor bx, bx
    mov di, buffer

.search_kernel:
    mov si, kernel_bin
    mov cx, 11
    push di
    repe cmpsb
    pop di
    je .found_kernel

    add di, 32
    inc bx
    cmp bx, [dir_entries]
    jl .search_kernel

    jmp kernel_not_found_error

.found_kernel:
    mov ax, [di + 26]
    mov [kernel_cluster], ax

    mov ax, [reserved_sectors]
    mov bx, buffer
    mov cl, [sectors_per_fat]
    mov dl, [drive_number]
    call disk_read

    mov bx, KERNEL_LOAD_SEGMENT
    mov es, bx
    mov bx, KERNEL_LOAD_OFFSET

.load_kernel_loop:
    mov ax, [kernel_cluster]

    add ax, 31 ; why tf is this hardcoded
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
    and ax, 0x0FFF

.next_cluster:
    cmp ax, 0x0ff8
    jae .read_finished

    mov [kernel_cluster], ax
    jmp .load_kernel_loop

.read_finished:
    mov dl, [drive_number]

    mov ax, KERNEL_LOAD_SEGMENT
    mov ds, ax
    mov es, ax

    jmp KERNEL_LOAD_SEGMENT:KERNEL_LOAD_OFFSET

    jmp wait_to_reboot

    cli
    hlt

floppy_error:
    mov si, read_failed
    call puts
    jmp wait_to_reboot

kernel_not_found_error:
    mov si, kernel_not_found
    call puts
    jmp wait_to_reboot

wait_to_reboot:
    mov ah, 0
    int 16h
    jmp 0ffffh:0

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

lba_to_chs:
    push ax
    push dx

    xor dx, dx
    div word [sectors_per_track]

    inc dx
    mov cx, dx

    xor dx, dx
    div word [heads]

    mov dh, dl
    mov ch, al
    shl ah, 6
    or cl, ah

    pop ax
    mov dl, al
    pop ax
    ret

disk_read:
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
    call disk_reset

    dec di
    test di, di
    jnz .retry

.fail:
    jmp floppy_error

.done:
    popa

    pop di
    pop dx
    pop cx
    pop bx
    pop ax

    ret

disk_reset:
    pusha

    mov ah, 0
    stc
    int 13h
    jc floppy_error

    popa
    ret

loading: db 'Loading...', ENDL, 0
read_failed: db 'Failed to read from disk', ENDL, 0
kernel_not_found: db 'Kernel not found', ENDL, 0
kernel_bin: db 'KERNEL  BIN'
kernel_cluster: dw 0

KERNEL_LOAD_SEGMENT equ 0x2000
KERNEL_LOAD_OFFSET equ 0

times 510-($-$$) db 0
dw 0aa55h

buffer: