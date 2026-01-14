@echo ON
if exist build\* del build\* /Q
if not exist build mkdir build

REM 1. Assemble trampoline (RAW BIN)
"%USERPROFILE%/AppData/Local/bin/NASM/nasm.exe" -f elf32 kernel\trampoline.asm -o build\trampoline.o

REM 2. Assemble GDT Flush (ELF32)
"%USERPROFILE%/AppData/Local/bin/NASM/nasm.exe" -f elf32 kernel\gdt_flush.asm -o build\gdt_flush.o

REM 3. Assemble kernel entry (ELF32)
"%USERPROFILE%/AppData/Local/bin/NASM/nasm.exe" -f elf32 kernel\kernel_entry.asm -o build\kernel_entry.o


REM 4. Compile C Files
call cross_compiler\i686-elf.gcc.bat -ffreestanding -m32 -c -fno-pic -fno-pie -nostdlib -O0 kernel\kernel.c -o build\kernel.o
call cross_compiler\i686-elf.gcc.bat -ffreestanding -m32 -c -fno-pic -fno-pie -nostdlib -O0 kernel\gdt.c -o build\gdt.o

REM 5. Link kernel ELF (Include all object files)
REM 5. Link kernel ELF
call cross_compiler\i686-elf-ld.bat -m elf_i386 -T linker.ld ^
  -o build\kernel.elf ^
  build\trampoline.o ^
  build\kernel_entry.o ^
  build\kernel.o

REM 6. Convert ELF → raw kernel binary
call cross_compiler\i686-elf-objcopy.bat -O binary build\kernel.elf outs\kernel.bin


echo Kernel build complete