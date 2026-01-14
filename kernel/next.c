// Usage: clrscr_ext(0x4F); // Clear screen to White on Red
void next(void) {
    volatile char* vga = (volatile char*)0xB8000;

    // By declaring it this way, the characters are pushed onto the 
    // stack as immediate values, avoiding the .rodata section issue.
    char msg[] = "Next in line::";
    
    int pos = 0; // Starting at index 8 (the 5th character cell)

    for (int i = 0; msg[i] != 0; i++) {
        vga[pos++] = msg[i];   // Write Character
        vga[pos++] = 0x0F;     // Write Attribute (White on Black)
    }

}