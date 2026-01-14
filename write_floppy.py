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
        # second_stage.bin should be exactly one 512-byte sector. If it's smaller,
        # pad with zeros; if larger, truncate to 512 bytes. This handles both
        # ORG-padded binaries and compact binaries.
        if len(data)!=512:
            raise ValueError(f'second_stage.bin must be exactly 512 bytes, but is {len(data)} bytes long.')

        print(f'seeking to {SECOND_STAGE_START - START_BOOT=}')
        f.seek((SECOND_STAGE_START - START_BOOT))  # Second sector offset in floppy
        f.write(data)

    # Write kernel image after second stage (starting at sector 3)
    # kernel.bin is a flat binary built by the linker and should be placed right after stage2
    import os
    if os.path.exists("outs/kernel.bin"):
        with open("outs/kernel.bin", "rb") as k:
            data = k.read()

            # pad kernel to full sector
            if len(data) % 512 != 0:
                data += b'\x00' * (512 - (len(data) % 512))

            f.seek(1024)  # sector 3
            f.write(data)
    else:
        print('Warning: outs/kernel.bin not found; kernel will not be written to floppy.')

print("Floppy disk successfully written!")
