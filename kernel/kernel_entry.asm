[BITS 32]
section .text
global kernel_entry
extern kernel_main

kernel_entry:
    cli

    mov byte [0xB8004], 'K'
    mov byte [0xB8005], 0x0F

    call kernel_main

.hang:
    cli
    hlt
    jmp .hang
