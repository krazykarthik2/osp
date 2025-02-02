[BITS 16]
[ORG 0x7E00]  ; BIOS loads us here
start:
    mov al,"?"
    call print_string
    mov al, 0xd
    call print_string
    mov al, 0xa
    call print_string
    hlt

print_string:
    mov ah, 0x0E
    mov bh, 0x00
    int 0x10
    ret

times 510 - ($ - $$) db 0
dw 0xAA55  ; Boot signature
