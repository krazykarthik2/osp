[BITS 16]
[ORG 0x9000]  ; BIOS loads us here

start:
    mov si, msg
    call print_string
    

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



msg db "SECOND-STAGE-AWAITS-YOU",0xd,0xa,0,
times 510 - ($ - $$) db 0
dw 0xAA55  ; Boot signature
