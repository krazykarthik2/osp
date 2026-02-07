@echo ON
if exist build\* del build\* /Q
if not exist build mkdir build

"%USERPROFILE%/AppData/Local/bin/NASM/nasm.exe" -f elf32 kernel\trampoline.asm -o build\trampoline.o
"%USERPROFILE%/AppData/Local/bin/NASM/nasm.exe" -f elf32 kernel\kernel_entry.asm -o build\kernel_entry.o
"%USERPROFILE%/AppData/Local/bin/NASM/nasm.exe" -f elf32 kernel\arch\i386\gdt_flush.asm -o build\gdt_flush.o
"%USERPROFILE%/AppData/Local/bin/NASM/nasm.exe" -f elf32 kernel\arch\i386\switch.asm -o build\switch.o
"%USERPROFILE%/AppData/Local/bin/NASM/nasm.exe" -f elf32 kernel\arch\i386\isr_stub.asm -o build\isr_stub.o


REM 4. Compile C files to ELF32 object files
set CFLAGS=-ffreestanding -m32 -c -fno-pic -fno-pie -nostdlib -O0 -I include

call cross_compiler\i686-elf.gcc.bat %CFLAGS% kernel\kernel.c -o build\kernel.o
call cross_compiler\i686-elf.gcc.bat %CFLAGS% kernel\next.c -o build\next.o
call cross_compiler\i686-elf.gcc.bat %CFLAGS% kernel\pic.c -o build\pic.o
call cross_compiler\i686-elf.gcc.bat %CFLAGS% kernel\console.c -o build\console.o
call cross_compiler\i686-elf.gcc.bat %CFLAGS% kernel\kbd.c -o build\kbd.o
call cross_compiler\i686-elf.gcc.bat %CFLAGS% kernel\sched.c -o build\sched.o
call cross_compiler\i686-elf.gcc.bat %CFLAGS% kernel\shell.c -o build\shell.o
call cross_compiler\i686-elf.gcc.bat %CFLAGS% kernel\log.c -o build\log.o
call cross_compiler\i686-elf.gcc.bat %CFLAGS% kernel\panic.c -o build\panic.o
call cross_compiler\i686-elf.gcc.bat %CFLAGS% kernel\arch\i386\gdt.c -o build\gdt.o
call cross_compiler\i686-elf.gcc.bat %CFLAGS% kernel\arch\i386\interrupts.c -o build\interrupts.o

:: 5. Link kernel ELF
call cross_compiler\i686-elf-ld.bat -m elf_i386 -T linker.ld ^
  -o build\kernel.elf ^
  build\trampoline.o ^
  build\kernel_entry.o ^
  build\kernel.o ^
  build\next.o ^
  build\interrupts.o ^
  build\gdt_flush.o ^
  build\switch.o ^
  build\gdt.o ^
  build\pic.o ^
  build\isr_stub.o ^
  build\console.o ^
  build\kbd.o ^
  build\sched.o ^
  build\shell.o ^
  build\log.o ^
  build\panic.o

REM 6. Convert ELF → raw kernel binary
call cross_compiler\i686-elf-objcopy.bat --change-addresses -0x10000 -O binary build\kernel.elf outs\kernel.bin


echo Kernel build complete
