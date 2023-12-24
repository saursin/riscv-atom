#h# ***** Makefile to build RISC-V Atom bootloader *****
#h# Usage: make soctarget=[SOCTARGET] [TARGET]

# soctarget to build for
soctarget?= atombones

# Build for simulation
sim?=1

# Build with prints enabled (if SoC has console)
prints?=1

_mk_check_env:=1
_mk_check_soctarget:=1
include ../../common.mk
##############################################################################

EXEC:= bootloader
SRCS:= main.c crt0.S target/$(soctarget)/boot.c

RVPREFIX:= riscv64-unknown-elf
CFLAGS:= -march=$(shell cfgparse.py $(RVATOM)/rtl/config/$(soctarget).json -a isa)
CFLAGS+= -mabi=$(shell cfgparse.py $(RVATOM)/rtl/config/$(soctarget).json -a abi)
CFLAGS+= -nostartfiles -ffreestanding -g -Os
CFLAGS+= -I $(RVATOM_LIB)/include -I include
CFLAGS+= $(shell cfgparse.py $(RVATOM)/rtl/config/$(soctarget).json -d)
CFLAGS+= -DTARGET_$(shell echo $(soctarget) | tr 'a-z' 'A-Z')
ifeq ($(sim), 1) 
    CFLAGS+= -DSIM
endif

ifeq ($(prints), 1)
    CFLAGS+= -DEN_PRINTS
endif

# This creates new section for each function which can be used with 
# -Wl,--gc-sections linker flag to remove unused code
CFLAGS+= -fdata-sections -ffunction-sections

LFLAGS:= -L $(RVATOM_LIB)/ -T $(RVATOM_LIB)/link/link_bootloader.ld -lcatom -Xlinker -Map $(EXEC).map -Wl,--gc-sections

##############################################################################
# Targets
default: build 

.PHONY: build
build: $(EXEC).elf			#t# Build bootloader

$(EXEC).elf: $(SRCS)
	$(call print_info, Building bootloader for soctarget: $(soctarget))
	$(RVPREFIX)-gcc $(CFLAGS) -o $@ $^ $(LFLAGS)
	$(RVPREFIX)-objdump -htd $@ > $(basename $@).lst
	convelf.py -t elf -c -m="BOOTROM:0x00010000:8192:h:$(basename $@).hex" $@

.PHONY: clean
clean:						#t# Clean build files
	$(call print_info, Cleaning bootloader)
	rm -f *.o *.lst *.map *.elf *.hex *.bin
