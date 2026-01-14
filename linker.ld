ENTRY(trampoline_start)

SECTIONS
{
    . = 0;

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
