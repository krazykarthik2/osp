ENTRY(trampoline_start)

SECTIONS
{
    /* Kernel is loaded at physical 0x10000 by the second-stage loader */
    . = 0x10000;

    .trampoline : {
        *(.trampoline)
    }

    .text : {
        *(.text*)
    }

    .rodata : {
        *(.rodata*)
    }

    .data : {
        *(.data*)
    }

    .bss : {
        *(.bss*)
    }
}
