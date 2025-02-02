BITS 16
;;;;;;;;;;;;constants up
;print just ? using direct addressing
; ORG 0x0000

; section .text
; global _start
; _start:
;     call print_char
;     mov eax, 1            ; System call number for sys_exit
;     xor ebx, ebx          ; Return code 0
;     int 0x80              ; Call kernel
; print_char:   
;     mov ah, 0x0E
;     mov al, 's'
;     int 0x10
;     ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;up:print ? direct addressing
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;down:print Hello, World! using memory pointers

[ORG 0x7E00]   ; Second stage starts at 0x7E00 (512 bytes after bootloader)


section .text
global _start
_start:
	mov	edx, len_msg   ;message length
	mov	ecx, hello_msg    ;message to write
	mov	ebx, 1	    ;file descriptor (stdout)
	mov	eax, 4	    ;system call number (sys_write)
	int	0x80        ;call kernel
    mov eax, 1            ; System call number for sys_exit
    xor ebx, ebx          ; Return code 0
    int 0x80              ; Call kernel
hello_msg db "Hello, World!", 0  ; Null-terminated string
len_msg equ $ - hello_msg        ; Length of the message

times 2020 - ($ - $$) db 0  ; Fill the rest of the sector with zeros
