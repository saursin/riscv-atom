########################################
src = 
bin_dir = .


############ ADVANCED ##################
# Directories
linker_script_path = lib/boot/linker.ld
bootloader_path = lib/boot/boot.s
bin_file = $(bin_dir)/a.elf

# Compiler options
CC = riscv64-unknown-elf-gcc
march = rv32i
mabi = ilp32
CC_flag = -nostdlib
########################################


# Barebone assembly without any additional code
.PHONY: asm_barebone
asm_barebone:
	 $(CC) -march=$(march) -mabi=$(mabi) $(CC_flag) -T $(linker_script_path) $(src) -o $(bin_file)

# Assembly with bootloader
.PHONY: asm
asm:
	 $(CC) -march=$(march) -mabi=$(mabi) $(CC_flag) -T $(linker_script_path) $(bootloader_path) $(src) -o $(bin_file)

# C with bootloader
.PHONY: c
c:
	 $(CC) -march=$(march) -mabi=$(mabi) $(CC_flag) -T $(linker_script_path) $(bootloader_path) $(src) -o $(bin_file)
