@echo OFF
del /s /f /q outs
@echo ON
"%USERPROFILE%/AppData/Local/bin/NASM/nasm.exe" -f bin boot.asm -o outs/boot.bin
"%USERPROFILE%/AppData/Local/bin/NASM/nasm.exe" -f bin second_stage.asm -o outs/second_stage.bin
@echo OFF
@echo 333333333333333333333333333333333333
fsutil file createnew "outs/floppy.img" 1474560 
@echo 444444444444444444444444444444444444
python write_floppy.py
@echo 5555555555555555555555555555555555555
@echo OFF
if not exist "outs/floppy.img" goto error
@echo ON
"C:\Program Files\Bochs-2.8\bochs.exe" -f bochsrc.txt -q
@echo OFF
goto end 
:error
@echo ON
@echo "Error: floppy.img not found, Exiting..."
:end
@echo ON
@echo dying....................