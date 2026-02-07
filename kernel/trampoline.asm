[BITS 16]
section .trampoline
global trampoline_start
extern kernel_entry

trampoline_start:
    ; --- Real-mode proof ---
    mov ax, 0xB800
    mov es, ax
    mov byte [es:0], 'T'
    mov byte [es:1], 0x2F

    cli
    cld
    xor ax, ax
    mov ds, ax

    ; 1. Mask the PIC to prevent hardware interrupts from firing
    mov al, 0xFF
    out 0x21, al
    out 0xA1, al

    ; 2. Setup GDTR (Absolute Physical Address)
    mov ebx, gdt
    a32 mov [gdtr + 2], ebx
    a32 mov word [gdtr], gdt_end - gdt - 1
    a32 lgdt [gdtr]

    ; 3. Setup IDTR (Absolute Physical Address)
    mov ebx, idt
    a32 mov [idtr + 2], ebx
    a32 mov word [idtr], 8*1 - 1
    a32 lidt [idtr]

    ; 4. Enter Protected Mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; 5. FAR jump to flush prefetch queue (physical address, 32-bit offset)
    mov ebx, pm_entry
    a32 mov [pm_phys_addr], ebx
    jmp short pm_jump

    ; 32-bit far jump opcode (operand-size override + far jump)
pm_jump:
    db 0x66, 0xEA
pm_phys_addr:
    dd 0
    dw 0x08

[BITS 32]
pm_entry:
    ; Load data segments
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    ; Build a minimal 32-entry IDT (exceptions) to avoid triple fault
    call .get_eip
.get_eip:
    pop ebx                        ; EBX = physical address of .get_eip
    sub ebx, (.get_eip - pm_entry) ; EBX = physical address of pm_entry
    mov esi, ebx
    sub esi, pm_entry              ; ESI = load base (physical)

    mov edi, esi
    add edi, idt32                 ; EDI = physical address of idt32
    mov edx, esi
    add edx, isr_stub32            ; EDX = physical address of handler

    mov ecx, 32
.idt_loop:
    mov eax, edx
    mov word [edi], ax         ; offset low
    mov word [edi+2], 0x08     ; selector
    mov byte [edi+4], 0
    mov byte [edi+5], 0x8E     ; present, ring0, 32-bit interrupt gate
    shr eax, 16
    mov word [edi+6], ax       ; offset high
    add edi, 8
    loop .idt_loop

    mov dword [idtr32 + 2], edi
    sub dword [idtr32 + 2], 32*8
    mov word [idtr32], (32*8 - 1)
    lidt [idtr32]

    ; --- Protected-mode proof ---
    mov byte [0xB8002], 'P'
    mov byte [0xB8003], 0x4F

    call kernel_entry

.hang:
    cli
    hlt
    jmp .hang

[BITS 32]
isr_stub:
    iretd

[BITS 32]
isr_stub32:
    cli
.halt:
    hlt
    jmp .halt

[BITS 16]
align 8
gdt:
    dq 0x0000000000000000
    dq 0x00CF9A000000FFFF   ; Code 0x08
    dq 0x00CF92000000FFFF   ; Data 0x10
gdt_end:

gdtr:
    dw 0
    dd 0

align 8
idt:
    ; Single entry for Vector 0
    dw 0 ; (Calculated at runtime usually, but we'll leave it blank for now)
    dw 0x08
    db 0
    db 0x8E
    dw 0

idtr:
    dw 0
    dd 0

[BITS 32]
align 8
idt32:
    times 32 dq 0

idtr32:
    dw 0
    dd 0
    
