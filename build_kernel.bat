@echo ON
if exist build\* del build\* /Q
if not exist build mkdir build

"%USERPROFILE%/AppData/Local/bin/NASM/nasm.exe" -f elf32 kernel\trampoline.asm -o build\trampoline.o
"%USERPROFILE%/AppData/Local/bin/NASM/nasm.exe" -f elf32 kernel\kernel_entry.asm -o build\kernel_entry.o
"%USERPROFILE%/AppData/Local/bin/NASM/nasm.exe" -f elf32 kernel\arch\i386\gdt_flush.asm -o build\gdt_flush.o


REM 4. Compile C files to ELF32 object files
set CFLAGS=-ffreestanding -m32 -c -fno-pic -fno-pie -nostdlib -O0 -I include

call cross_compiler\i686-elf.gcc.bat %CFLAGS% kernel\kernel.c -o build\kernel.o
call cross_compiler\i686-elf.gcc.bat %CFLAGS% kernel\next.c -o build\next.o
call cross_compiler\i686-elf.gcc.bat %CFLAGS% kernel\arch\i386\gdt.c -o build\gdt.o
call cross_compiler\i686-elf.gcc.bat %CFLAGS% kernel\arch\i386\interrupts.c -o build\interrupts.o

:: 5. Link kernel ELF
call cross_compiler\i686-elf-ld.bat -m elf_i386 -T linker.ld ^
  -o build\kernel.elf ^
  build\trampoline.o ^
  build\kernel_entry.o ^
  build\kernel.o ^
  build\next.o ^
  build\gdt_flush.o ^
  build\gdt.o ^
  build\interrupts.o 

REM 6. Convert ELF → raw kernel binary
call cross_compiler\i686-elf-objcopy.bat -O binary build\kernel.elf outs\kernel.bin


echo Kernel build complete