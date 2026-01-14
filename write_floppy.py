START_BOOT = 0x7C00  #DON'T CHANGE THIS VALUE
SECOND_STAGE_START = 0x7c00 + 0x200
with open("outs/floppy.img", "r+b") as f:
    # Write first stage (512 bytes) at sector 1
    with open("outs/boot.bin", "rb") as boot:
        f.seek(0)  # First sector
        f.write(boot.read())
    # Write second stage at sector 2
    with open("outs/second_stage.bin", "rb") as second_stage:
        data = second_stage.read()
        # second_stage.bin produced by NASM with `ORG 0x7E00` will be a large file
        # containing zeros up to address 0x7E00; extract the 512-byte sector
        start = SECOND_STAGE_START
        sector = data[start:start+0x200]
        if len(sector) < 0x200:
            print(f'Warning: second_stage.bin too small ({len(data)} bytes). Writing available bytes.')
        print(f'seeking to {SECOND_STAGE_START - START_BOOT=}')
        f.seek((SECOND_STAGE_START - START_BOOT))  # Second sector offset in floppy
        f.write(sector)

    # Write kernel image after second stage (starting at sector 3)
    # kernel.bin is a flat binary built by the linker and should be placed right after stage2
    import os
    if os.path.exists("build/kernel.bin"):
        with open("build/kernel.bin", "rb") as k:
            # seek to third sector (2 * 512 bytes after START_BOOT)
            third_sector_offset = (SECOND_STAGE_START - START_BOOT) + 0x200
            f.seek(third_sector_offset)
            f.write(k.read())
    else:
        print('Warning: build/kernel.bin not found; kernel will not be written to floppy.')

print("Floppy disk successfully written!")
