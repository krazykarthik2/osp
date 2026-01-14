[BITS 32]
section .text
global gdt_flush

gdt_flush:
    ; [esp + 4] is the physical address of the GDT array
    mov eax, [esp + 4]

    ; --- DEBUG: '1' Entered ---
    mov byte [0xB8000], '1'
    mov byte [0xB8001], 0x0F

    ; Build GDT Descriptor on stack: [ 4-byte Base ][ 2-byte Limit ]
    push eax            ; Push 32-bit Physical Base
    push word 23        ; Push 16-bit Limit (3 entries * 8 bytes - 1)

    ; --- DEBUG: '2' Ready to Load ---
    mov byte [0xB8002], '2'
    mov byte [0xB8003], 0x0F

    lgdt [esp]          ; LOAD GDT from the stack
    add esp, 6          ; Clean up the 6 bytes we pushed

    ; --- DEBUG: '3' LGDT Instruction Passed ---
    mov byte [0xB8004], '3'
    mov byte [0xB8005], 0x0A ; Green

    ; Reload Data Segments
    mov ax, 0x10        ; Data Selector (0x10)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax          ; If it crashes here, the 0x92 entry is wrong

; --- DEBUG: 'D' Data segments reloaded ---
    mov byte [0xB8006], 'D'
    mov byte [0xB8007], 0x0B ; Cyan

    ; We need the PHYSICAL address of the code below to jump to it.
    ; We calculate it exactly like we did in the trampoline.
    call .get_phys
.get_phys:
    pop eax             ; EAX now holds the physical address of .get_phys
    add eax, (.reload_cs - .get_phys) ; Calculate physical address of .reload_cs

    ; Manual Far Jump using the calculated physical address
    push 0x08           ; Push Code Selector
    push eax            ; Push Physical Address of .reload_cs
    retf                ; Jump!

.reload_cs:
    ; --- DEBUG: '4' CS reloaded successfully ---
    mov byte [0xB8008], '4'
    mov byte [0xB8009], 0x0F
    ret