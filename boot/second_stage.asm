[BITS 16]
[ORG 0x7E00]

start:
    mov [boot_drive], dl

    cli
    xor ax, ax
    mov ds, ax          ; DS = 0 (correct for ORG 7E00)
    mov ss, ax
    mov sp, 0x9C00      ; ✅ FIXED STACK
    sti

    mov si, hello_msg
    call print_string

    call load_kernel

    mov si, kernel_loaded_msg
    call print_string

    mov ah, 0x0E
    mov al, 'P'
    int 0x10
    cli
    jmp 0x1000:0x0000

; -----------------------------

load_kernel:
    pusha

    ; reset disk system (IMPORTANT)
    mov ah, 0x00
    mov dl, [boot_drive]
    int 0x13

    ; destination: physical 0x10000
    mov ax, 0x1000
    mov es, ax
    xor bx, bx

    xor ch, ch        ; cylinder 0
    xor dh, dh        ; head 0
    mov cl, 3         ; sector 3

    mov bp, 2       ; number of sectors to read

.read:
    mov dl, [boot_drive]
    mov ah, 0x02
    mov al, 1
    int 0x13
    jc disk_error

    add bx, 512       ; REQUIRED if bp > 1
    inc cl
    dec bp
    jnz .read

    popa
    ret


disk_error:
    popa
    mov si, disk_err_msg
    call print_string
.hang:
    cli
    hlt
    jmp .hang

print_string:
    mov ah, 0x0E
.next:
    lodsb               ; DS:SI → absolute address
    test al, al
    jz .done
    int 0x10
    jmp .next
.done:
    ret

boot_drive db 0
hello_msg db "Hello from Stage 2!", 0x0D, 0x0A, 0
kernel_loaded_msg db "Kernel loaded. Jumping...", 0x0D, 0x0A, 0
disk_err_msg db "Disk read error!", 0x0D, 0x0A, 0

times 512 - ($ - $$) db 0
