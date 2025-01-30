with open("floppy.img", "r+b") as f:
    # Write first stage (512 bytes) at sector 1
    with open("boot.img", "rb") as boot:
        f.seek(0)  # First sector
        f.write(boot.read())

    # Write second stage at sector 2
    with open("second_stage.img", "rb") as second_stage:
        f.seek(0x9000)  # Second sector
        f.write(second_stage.read())

print("Floppy disk successfully written!")
