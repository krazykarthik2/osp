[BITS 16]
[ORG 0x7C00]  ; BIOS loads us here

start:
    cli              ; Disable interrupts
    xor ax, ax
    mov ds, ax
    mov es, ax
    sti              ; Enable interrupts

    mov si, msg
    call print_string

    mov ax, 0x9000   ; Load into segment 0x9000 (0x90000)
    mov es, ax
    mov bx, 0x0002   ; Offset 0x0000
    mov ah, 0x02     ; BIOS disk read function
    mov al, 1        ; Read 1 sectors
    mov ch, 0        ; Cylinder 0
    mov dh, 0        ; Head 0
    mov cl, 2       ; Start at sector 2
    int 0x13         ; Call BIOS disk interrupt
    jc disk_error    ; If error, print error message

    mov si, 0x90000
    call print_hex 
    mov si, 0x90001
    call print_hex 
    mov si, 0x90002
    call print_hex 
    mov si, 0x90003
    call print_hex 
    mov si, 0x90004
    call print_hex 
    mov si, 0x90005
    call print_hex 

    mov si, jumping
    call print_string

    mov si, success_msg
    call print_string
    jmp 0x90000:0x0000  ; Jump to second-stage bootloader

    success_msg db "Loaded Stage 2 into memory", 0x0D, 0x0A, 0

disk_error:
    mov si, err_msg
    call print_string
    hlt

print_hex:
    add al, 0x30
    cmp al, 0x39
    call print_char
    add al, 0x07
    mov si,newline
    call print_char


print_char:
    mov ah, 0x0E        ; BIOS teletype output
    int 0x10
    ret



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


msg db "Started Boot.asm,",0x0D,0x0A,0,
jumping db "Jumping to second stage...", 0x0D, 0x0A, 0
overthrowed db "Overthrowed", 0x0D, 0x0A, 0
err_msg db "Disk read error!", 0x0D, 0x0A, 0
newline db 0x0D, 0x0A, 0
times 510 - ($ - $$) db 0
dw 0xAA55  ; Boot signature
