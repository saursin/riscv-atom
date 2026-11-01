#h# ***** Makefile to build RISC-V Atom bootloader *****

#v# Enable prints in bootloader
prints?=1

_mk_check_env:=1
_mk_check_soctarget:=1
include ../../common.mk
##############################################################################

_BOOT_DIR := $(RVATOM_BUILD)/sw/bootloader
$(shell mkdir -p $(_BOOT_DIR))

EXEC:= $(_BOOT_DIR)/bootloader
SRCS:= crt0.S main.c 
SRCS+= $(wildcard target/$(soctarget)/*.c)
SRCS+= $(wildcard target/$(soctarget)/*.S)

CFLAGS    := -march=$(RVATOM_ISA)
CFLAGS    += -mabi=$(RVATOM_ABI)
CFLAGS    += -Wall -nostartfiles -ffreestanding -g -Os
CFLAGS    += -I $(RVATOM_LIB)/include -I include -I target/$(soctarget)
CFLAGS    += -include $(RVATOM_CONFIG_H)	# config defines (memory map, target) from the YAML

ifeq ($(prints), 1)
    CFLAGS+= -DEN_PRINTS
endif

# This creates new section for each function which can be used with 
# -Wl,--gc-sections linker flag to remove unused code
CFLAGS+= -fdata-sections -ffunction-sections
# link-time optimization across the bootloader's files
CFLAGS+= -flto

# libcatom is the C library: no toolchain libc (-nostdlib), only libgcc helpers
LFLAGS:= -nostdlib -L $(RVATOM_BUILD)/sw/lib -T $(RVATOM_BUILD)/sw/lib/link_bootloader.ld -lcatom -lgcc -Xlinker -Map $(EXEC).map -Wl,--gc-sections

##############################################################################
# Targets
default: build 

.PHONY: build
build: $(EXEC).elf			#t# Build bootloader

# Platform table (clock, ...) from the soctarget/board config
PLATFORM_SRC:= $(_BOOT_DIR)/platform_table.c
$(PLATFORM_SRC): $(RVATOM_VARS_MK)
	$(CFGPARSE) -c $(RVATOM_YMLCONFIG) -t platform -o $@

$(EXEC).elf: $(SRCS) $(PLATFORM_SRC) $(RVATOM_BUILD)/sw/lib/libcatom.a $(RVATOM_VARS_MK)
	$(call print_msgt,Compiling)
	$(CC) $(CFLAGS) -MM -MP -MT $@ $(SRCS) > $(EXEC).d	# header dependencies
	$(CC) $(CFLAGS) -o $@ $(SRCS) $(PLATFORM_SRC) $(LFLAGS)
	$(OBJDUMP) -htD $@ > $(basename $@).lst
	$(call print_msg,Generating hex,$(basename $@).hex)
	convelf.py -t elf -c -m="BOOTROM:$(RVATOM_BOOTROM_ADDR):$(RVATOM_BOOTROM_SIZE):h:$(basename $@).hex" $@

.PHONY: clean
clean:						#t# Clean build files
	rm -f $(EXEC).* $(PLATFORM_SRC)

-include $(EXEC).d
