[BITS 32]
section .text
global switch_context

; void switch_context(uint32_t* old_esp, uint32_t new_esp)
; - Saves current context onto current stack (pushad)
; - Stores resulting ESP into *old_esp
; - Loads ESP = new_esp
; - Restores context (popad) and returns into new task
switch_context:
    pushad
    mov eax, [esp + 32 + 4]   ; old_esp (after pushad)
    mov [eax], esp
    mov edx, [esp + 32 + 8]   ; new_esp
    mov esp, edx
    popad
    ret
