ENTRY(trampoline_start)

SECTIONS
{
    /* Change this to the actual address where the file is loaded in RAM */
    . = 0x00; 

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