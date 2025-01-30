@echo ON
"%USERPROFILE%/AppData/Local/bin/NASM/nasm.exe" -f bin boot.asm -o boot.img
"%USERPROFILE%/AppData/Local/bin/NASM/nasm.exe" -f bin second_stage.asm -o second_stage.img
@echo OFF
if exist "floppy.img" del /f /q floppy.img 
fsutil file createnew floppy.img 1474560 
python write_floppy.py
@echo OFF
if not exist "floppy.img" goto error
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