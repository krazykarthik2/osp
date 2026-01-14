extern void next(void);
extern void gdt_install(void);
extern void idt_install(void);

void clrscr_ext(unsigned char color) {
    volatile unsigned short* vga = (volatile unsigned short*)0xB8000;
    // Shift color to high byte and add space character
    unsigned short field = (color << 8) | 0x20; 

    for (int i = 0; i < 80 * 25; i++) {
        vga[i] = field;
    }
}

// Usage: clrscr_ext(0x4F); // Clear screen to White on Red
void kernel_main(void) {
    clrscr_ext(0xF0); // Clear screen to Black on White
    // delay for 100 milliseconds
    for (volatile int i = 0; i < 1000000*100; i++) {
        __asm__ __volatile__("nop");
    }
    clrscr_ext(0x0F); // Clear screen to White on Black
    volatile char* vga = (volatile char*)0xB8000;

    // By declaring it this way, the characters are pushed onto the 
    // stack as immediate values, avoiding the .rodata section issue.
    char msg[] = "C programming in Kernel!";
    
    int pos = 0; // Starting at index 8 (the 5th character cell)

    for (int i = 0; msg[i] != 0; i++) {
        vga[pos++] = msg[i];   // Write Character
        vga[pos++] = 0x0F;     // Write Attribute (White on Black)
    }

    // Clear screen to Black on White
    // delay for 100 milliseconds
    for (volatile int i = 0; i < 1000000*100; i++) {
        __asm__ __volatile__("nop");
    }
    next();
    
    gdt_install();
    idt_install();
    for (;;)
        __asm__ __volatile__("hlt");
}