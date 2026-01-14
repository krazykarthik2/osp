[BITS 16]
[ORG 0x7C00]          ; BIOS loads us here

start:
    ; ---- SAVE BOOT DRIVE IMMEDIATELY ----
    cli
    xor ax, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    mov [boot_drive], dl

    mov ax, cs
    mov ds, ax

    mov si, msg
    call print_string

    ; ---- LOAD SECOND STAGE TO 0x7E00 ----
    mov ax, 0x07C0
    mov es, ax
    mov bx, 0x0200        ; offset 0x200 → 0x7E00

    ; 🔴 RESET DISK SYSTEM (IMPORTANT)
    mov ah, 0x00
    mov dl, [boot_drive]
    int 0x13

    mov dl, [boot_drive] ; 🔴 RESTORE DL
    mov ah, 0x02
    mov al, 1
    mov ch, 0
    mov dh, 0
    mov cl, 2
    int 0x13
    jc disk_error

    mov si, jumping
    call print_string

    jmp 0x07C0:0x0200    ; jump to stage-2

disk_error:
    mov si, err_msg
    call print_string
.hang:
    cli
    hlt
    jmp .hang

; ----------------------------
; Print helpers
; ----------------------------
print_string:
    mov ah, 0x0E
.loop:
    lodsb
    test al, al
    jz .done
    int 0x10
    jmp .loop
.done:
    ret

; ----------------------------
; Data
; ----------------------------
boot_drive db 0

msg db "Started Boot.asm",0x0D,0x0A,0
jumping db "Jumping to second stage...",0x0D,0x0A,0
err_msg db "Disk read error!",0x0D,0x0A,0

times 510 - ($ - $$) db 0
dw 0xAA55
