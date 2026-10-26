#h# ***** Makefile to build RISC-V Atom bootloader *****

#v# Build for simulation
sim?=1

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
CFLAGS    += $(RVATOM_CDEFINES)
ifeq ($(sim), 1)
    CFLAGS+= -DSIM
endif

ifeq ($(prints), 1)
    CFLAGS+= -DEN_PRINTS
endif

# This creates new section for each function which can be used with 
# -Wl,--gc-sections linker flag to remove unused code
CFLAGS+= -fdata-sections -ffunction-sections

LFLAGS:= -L $(RVATOM_BUILD)/sw/lib -T $(RVATOM_LIB)/link/link_bootloader.ld -lcatom -Xlinker -Map $(EXEC).map -Wl,--gc-sections

##############################################################################
# Targets
default: build 

.PHONY: build
build: $(EXEC).elf			#t# Build bootloader

$(EXEC).elf: $(SRCS) $(RVATOM_VARS_MK)
	$(call print_msgt,Compiling)
	$(CC) $(CFLAGS) -o $@ $(SRCS) $(LFLAGS)
	$(OBJDUMP) -htD $@ > $(basename $@).lst
	$(call print_msg,Generating hex,$(basename $@).hex)
	convelf.py -t elf -c -m="BOOTROM:0x00010000:8192:h:$(basename $@).hex" $@

.PHONY: clean
clean:						#t# Clean build files
	rm -f $(EXEC).*
