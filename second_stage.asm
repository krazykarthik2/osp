[BITS 16]
[ORG 0x9000]

start:
    mov si, msg
    call print_string  ; Make sure this works
    hlt               ; Stop execution to debug

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
msg db "Second Stage Loaded!", 0x0D, 0x0A, 0
