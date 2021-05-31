########################################
## Change these variables
src = 
bin_dir = $(src_dir)
libpath = ../lib

############ ADVANCED ##################
## Directories
boot_path = $(libpath)/boot
linker_script_path = $(libboot_path)/link.ld

bootloader_path = $(libboot_path)/boot.s
bin_file = $(bin_dir)/a.elf

# Compiler options
CCPREFIX = riscv64-unknown-elf-
CC = $(CCPREFIX)gcc

march = rv32i
mabi = ilp32

CFLAG = -nostdlib
########################################


# Barebone assembly without any additional code
asm_barebone:
	 $(CC) -march=$(march) -mabi=$(mabi) $(CC_flag) -T $(linker_script_path) $(src) -o $(bin_file)

# Assembly with bootloader
asm:
	 $(CC) -march=$(march) -mabi=$(mabi) $(CC_flag) -T $(linker_script_path) $(bootloader_path) $(src) -o $(bin_file)

# C with bootloader
c:
	 $(CC) -march=$(march) -mabi=$(mabi) $(CC_flag) -T $(linker_script_path) $(bootloader_path) $(src) -o $(bin_file)
