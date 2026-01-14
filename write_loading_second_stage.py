# replace <No_of_disk_to_be_read_is_written_here> in second_stage.asm with number of disks to be read for loading the kernel. i.e, size of kernel.bin in sectors (1 sector = 512 bytes)
import os
kernel_path = "outs/kernel.bin"
second_stage_path = "boot/second_stage.asm"
second_stage_path_mediatory = "compilation_temp/second_stage.asm"
kernel_size = os.path.getsize(kernel_path)
sectors = (kernel_size + 511) // 512  # Round up to the nearest sector
with open(second_stage_path, "r") as f:
    content = f.read()
    content = content.replace("<No_of_disk_to_be_read_is_written_here>", str(sectors))
with open(second_stage_path_mediatory, "w") as f:
    f.write(content)
print(f"Replaced <No_of_disk_to_be_read_is_written_here> with {sectors} in {second_stage_path_mediatory}")