@echo OFF
REM Build kernel: compile C and assembly then link to flat kernel image
if not exist build mkdir build

REM Compile C (uses cross_compiler\i686-elf.gcc.bat as a wrapper)
call cross_compiler\i686-elf.gcc.bat -ffreestanding -m32 -c kernel\kernel.c -o build\kernel.o

REM Assemble entry
"%USERPROFILE%/AppData/Local/bin/NASM/nasm.exe" -f elf32 kernel\kernel_entry.asm -o build\kernel_entry.o

REM Link kernel (requires i686-elf-ld in PATH)
call cross_compiler/i686-elf-ld.bat -m elf_i386 -T linker.ld -o build\kernel.bin build\kernel_entry.o build\kernel.o

echo Kernel build complete: build\kernel.bin
