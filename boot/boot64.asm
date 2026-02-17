%define CR0_PE  (1 << 0)
%define CR0_PG  (1 << 31)
%define CR0_CD  (1 << 30)
%define CR0_NW  (1 << 29)
%define CR4_PAE (1 << 5)
%define EFER_MSR 0xC0000080
%define EFER_LME (1 << 8)

section .multiboot
align 8
mb2_header:
    dd 0xE85250D6
    dd 0
    dd mb2_header_end - mb2_header
    dd -(0xE85250D6 + 0 + (mb2_header_end - mb2_header))
    dw 0
    dw 0
    dd 8
mb2_header_end:

section .text
bits 32
global _start
extern kmain

_start:
    cli
    cmp eax, 0x36D76289
    jne .hang

    mov esp, stack_top32

    mov edi, page_table_l2
    xor eax, eax
    mov ecx, 512
.map_pd:
    mov ebx, eax
    or ebx, 0x83
    mov [edi], ebx
    add edi, 8
    add eax, 0x200000
    loop .map_pd

    mov eax, page_table_l2
    or eax, 0x3
    mov [page_table_l3], eax

    mov eax, page_table_l3
    or eax, 0x3
    mov [page_table_l4], eax

    mov eax, cr4
    or eax, CR4_PAE
    mov cr4, eax

    mov ecx, EFER_MSR
    rdmsr
    or eax, EFER_LME
    wrmsr

    mov eax, page_table_l4
    mov cr3, eax

    mov eax, cr0
    and eax, ~(CR0_CD | CR0_NW)
    or eax, (CR0_PE | CR0_PG)
    mov cr0, eax

    lgdt [gdt64.pointer]
    jmp gdt64.code:long_mode_start

.hang:
    hlt
    jmp .hang

bits 64
long_mode_start:
    mov ax, gdt64.data
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov rsp, stack_top64
    xor rbp, rbp
    call kmain

.dead:
    hlt
    jmp .dead

align 16
gdt64:
    dq 0
.code equ $ - gdt64
    dq 0x00AF9A000000FFFF
.data equ $ - gdt64
    dq 0x00AF92000000FFFF
.pointer:
    dw $ - gdt64 - 1
    dq gdt64

section .bss
align 4096
page_table_l4: resb 4096
page_table_l3: resb 4096
page_table_l2: resb 4096

align 16
stack_space: resb 16384
stack_top32:
stack_top64:
