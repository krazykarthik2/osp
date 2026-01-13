; Second stage loader
; Keep ORG at 0x7E00 and initialize segments to zero (this avoids memory errors on some setups)
[BITS 16]
[ORG 0x7e00]

start:
    cli
    xor ax, ax
    mov ss, ax
    mov ds, ax
    mov es, ax
    sti

    mov si, hello_msg
    call print_string

    ; Load kernel (started at sector 3 on floppy) into physical 0x10000 (ES:BX = 0x1000:0)
    call load_kernel

    mov si, kernel_loaded_msg
    call print_string

    ; Enter protected mode and jump to kernel at 0x10000
    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp 0x08:.protected_entry

; --------------------- loader ---------------------
load_kernel:
    pusha

    ; destination ES:BX = 0x1000:0 -> physical 0x10000
    mov ax, 0x1000
    mov es, ax
    xor bx, bx

    xor ch, ch    ; cylinder 0
    xor dh, dh    ; head 0
    mov cl, 3     ; start at sector 3

    mov bp, 100   ; max sectors to read (tunable)
.read_loop:
    mov ah, 0x02
    mov al, 1
    int 0x13
    jc .disk_error

    ; advance ES:BX by 512
    add bx, 512
    cmp bx, 0x1000
    jb .no_inc_es
    sub bx, 0x1000
    inc es
.no_inc_es:

    ; advance CHS: sectors per track = 18, heads = 2
    inc cl
    cmp cl, 19
    jl .cont
    mov cl, 1
    inc dh
    cmp dh, 2
    jl .cont
    mov dh, 0
    inc ch
.cont:

    dec bp
    jnz .read_loop

    popa
    ret

.disk_error:
    popa
    mov si, disk_err_msg
    call print_string
    hlt
    jmp .disk_error

; --------------------- protected entry ---------------------
; Switch to 32-bit and jump to kernel entry
bits 32
.protected_entry:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x9FC00
    ; jump to kernel linked to 0x10000
    mov eax, 0x10000
    jmp eax

; --------------------- GDT ---------------------
bits 16
gdt_start:
    dd 0x00000000
    dd 0x00000000

; code segment descriptor
gdt_code:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0x9A
    db 0xCF
    db 0x00

; data segment descriptor
gdt_data:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0x92
    db 0xCF
    db 0x00

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

; --------------------- helpers ---------------------
print_string:
    mov ah, 0x0E
.next:
    lodsb
    test al, al
    jz .done
    int 0x10
    jmp .next
.done:
    ret

hello_msg db "Hello from Stage 2!", 0x0D, 0x0A, 0
kernel_loaded_msg db "Kernel loaded; entering protected mode.", 0x0D, 0x0A, 0
disk_err_msg db "Disk read error while loading kernel!", 0x0D, 0x0A, 0