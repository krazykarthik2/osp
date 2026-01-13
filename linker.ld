ENTRY(_start)

SECTIONS
{
    . = 0x10000; /* Link at 64KB so the stage-2 loader can load the flat kernel image into real-mode-accessible memory */

    .text : { *(.text*) }
    .rodata : { *(.rodata*) }
    .data : { *(.data*) }
    .bss : { *(COMMON) *(.bss*) }
}
