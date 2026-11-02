#h# ***** RISC-V Atom example *****
# Rules shared by the examples. An example's Makefile names its sources and output, adds any
# CFLAGS of its own, and includes this file:
#     src_files  = hello.c
#     executable = hello.elf
#     include ../example.mk

#v# Connect the UART for run: pty (talk to the program with atomterm), or a serial port
vuart ?=

#v# Save a map file while compiling
map ?= 1

HELP_MAKEFILE := $(lastword $(MAKEFILE_LIST))
_mk_check_env = 1
_mk_check_soctarget = 1
include $(dir $(HELP_MAKEFILE))../../common.mk
.DEFAULT_GOAL := compile

RVPREFIX := riscv64-unknown-elf
# before the example's own CFLAGS, so those win (e.g. -O2)
ATOM_CFLAGS := -Wall -include $(RVATOM_CONFIG_H)
ATOM_CFLAGS += -march=$(RVATOM_ISA) -mabi=$(RVATOM_ABI) -nostartfiles -ffreestanding -Os -fdata-sections -ffunction-sections
ATOM_CFLAGS += -I $(RVATOM_LIB)/include -L $(RVATOM_BUILD)/sw/lib
# libcatom is the C library: no toolchain libc (-nostdlib), only libgcc helpers
LFLAGS := -nostdlib -T $(RVATOM_BUILD)/sw/lib/link_$(soctarget).ld -lcatom -lgcc -Wl,--gc-sections
ifeq ($(map), 1)
    LFLAGS += -Xlinker -Map $(basename $(executable)).map
endif

# atomsim of the selected soctarget (each target has its own binary, none is on PATH)
ATOMSIM := $(RVATOM_BUILD)/sim/bin/atomsim
ATOMSIM_ARGS = --maxitr=999999999 -uv
ifneq ($(vuart),)
    ATOMSIM_ARGS += --vuart-port=$(vuart)
endif


.PHONY: compile
compile: $(executable)						#t# Compile the example

$(executable): $(src_files) $(RVATOM_VARS_MK)
	$(call print_msgt,Building)
	$(RVPREFIX)-gcc $(ATOM_CFLAGS) $(CFLAGS) $(src_files) -o $@ $(LFLAGS)
	$(RVPREFIX)-objdump -Shtd $@ > $(basename $@).lst
	$(RVPREFIX)-objcopy -O binary $@ $(basename $@).bin
	@printf "$(CLR_BL)$(CLR_B)  Binary size:$(CLR_NB) %s bytes$(CLR_NC)\n" $$(stat --format=%s "$(basename $@).bin")

.PHONY: run
run: $(executable)							#t# Run the example in atomsim
	$(call print_msg,Running, $<)
	@test -x $(ATOMSIM) || { printf "$(CLR_RD)$(CLR_B)ERROR:$(CLR_NC) atomsim not built at $(ATOMSIM)\n"; exit 1; }
	$(ATOMSIM) $(ATOMSIM_ARGS) $<

.PHONY: dism
dism: $(executable)							#t# Dump disassembly
	$(call print_msg,Generating disassembly,$(basename $<).lst)
	$(RVPREFIX)-objdump -htd $< > $(basename $<).lst

.PHONY: clean
clean:										#t# Clean build outputs
	$(call print_msg,Cleaning,$(CURDIR))
	rm -f $(executable) $(basename $(executable)).bin *.o *.map *.lst
