[BITS 16]
section .trampoline
global trampoline_start

trampoline_start:
    ; --- Real-mode proof ---
    mov ax, 0xB800
    mov es, ax
    mov byte [es:0], 'T'
    mov byte [es:1], 0x2F

    cli
    cld

    ; 1. Mask the PIC to prevent hardware interrupts from firing
    mov al, 0xFF
    out 0x21, al
    out 0xA1, al

    ; 2. Calculate absolute physical address of this code
    ; This ensures GDT/IDT work regardless of where boot.asm loaded us
    xor eax, eax
    mov ax, cs
    shl eax, 4              ; EAX = Physical address of start of segment

    ; 3. Setup GDTR (Physical Address)
    mov ebx, eax
    add ebx, gdt            ; EBX = Physical address of GDT
    mov [gdtr + 2], ebx
    mov word [gdtr], gdt_end - gdt - 1
    lgdt [gdtr]

    ; 4. Setup IDTR (Physical Address)
    mov ebx, eax
    add ebx, idt            ; EBX = Physical address of IDT
    mov [idtr + 2], ebx
    mov word [idtr], 8*1 - 1
    lidt [idtr]

    ; 5. Enter Protected Mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; 6. 32-bit FAR jump
    ; We must calculate the absolute address of pm_entry for the jump
    ; Using a manual encoded far jump to be safe with relocations
    db 0x66, 0xEA           ; Far jump opcode

pm_phys_addr:
    dd 0                    ; Will be filled below
    dw 0x08                 ; Code Selector

; Fixup the jump address dynamically before jumping
    mov ebx, cs
    shl ebx, 4
    add ebx, pm_entry
    mov [pm_phys_addr], ebx

    ; Perform the jump (we actually just need to point to it)
    ; Since the above is a 'db' block, we just execute into it or 
    ; use a register indirect jump. Here is the cleaner way:
    push 0x08               ; Selector
    push ebx                ; Physical offset of pm_entry
    retf                    ; 32-bit far "jump" via return

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

    ; --- Protected-mode proof ---
    mov byte [0xB8002], 'P'
    mov byte [0xB8003], 0x4F

    jmp $

[BITS 32]
isr_stub:
    iretd

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
    