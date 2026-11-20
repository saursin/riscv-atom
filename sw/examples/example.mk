#h# ***** RISC-V Atom example *****
# Rules shared by the examples. An example's Makefile names its sources and output, adds any
# CFLAGS of its own, and includes this file:
#     src_files  = hello.c
#     executable = hello.elf
#     include ../example.mk
# An example that needs particular SoC targets (e.g. for GPIO) lists them before the include, as
#     soctargets = hydrogensoc
# and is skipped on other targets. An example that uses a library from sw/lib besides libcatom
# names it, also before the include:
#     libs = fatfs
# Outputs go to $(RVATOM_BUILD)/examples/<example>/ (e.g. build/hydrogensoc/examples/banner/), one
# folder per soctarget, so switching targets never reuses another target's build. The ELF is
# rebuilt when its sources, the headers they include or the libraries change.

#v# Connect the UART for run: pty (talk to the program with atomterm), or a serial port
vuart ?=

#v# Serial port for load (default: $RVATOM_PORT, else the simulator's sim.tty)
port ?= $(RVATOM_PORT)

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
ATOM_CFLAGS += -I $(RVATOM_LIB)/include $(addprefix -I $(RVATOM_LIB)/,$(libs)) -L $(RVATOM_BUILD)/sw/lib
# libcatom is the C library: no toolchain libc (-nostdlib), only libgcc helpers. Other libraries
# come first, as they call into libcatom.
LFLAGS := -nostdlib -T $(RVATOM_BUILD)/sw/lib/link_$(soctarget).ld $(addprefix -l,$(libs)) -lcatom -lgcc -Wl,--gc-sections

OUT  := $(RVATOM_BUILD)/examples/$(notdir $(CURDIR))
ELF  := $(OUT)/$(executable)
LIBS := $(RVATOM_BUILD)/sw/lib/libcatom.a $(foreach l,$(libs),$(RVATOM_BUILD)/sw/lib/lib$(l).a)
ifeq ($(map), 1)
    LFLAGS += -Xlinker -Map $(basename $(ELF)).map
endif

# atomsim of the selected soctarget (each target has its own binary, none is on PATH)
ATOMSIM := $(RVATOM_BUILD)/sim/bin/atomsim
ATOMSIM_ARGS = --maxitr=999999999 -uv
ifneq ($(vuart),)
    ATOMSIM_ARGS += --vuart-port=$(vuart)
endif

ifneq ($(soctargets),)
    ifeq ($(filter $(soctarget),$(soctargets)),)
        skip := 1
    endif
endif


ifeq ($(skip),1)
.PHONY: compile run load dism
compile run load dism:
	@printf "$(CLR_YL)  skipped:$(CLR_NC) $(notdir $(CURDIR)) needs soctarget $(soctargets)\n"
else

.PHONY: compile
compile: $(ELF)								#t# Compile the example

$(ELF): $(src_files) $(LIBS) $(RVATOM_VARS_MK)
	$(call print_msgt,Building)
	@mkdir -p $(OUT)
	$(RVPREFIX)-gcc $(ATOM_CFLAGS) $(CFLAGS) -MM -MP -MT $@ $(src_files) > $(OUT)/deps.d
	$(RVPREFIX)-gcc $(ATOM_CFLAGS) $(CFLAGS) $(src_files) -o $@ $(LFLAGS)
	$(RVPREFIX)-objdump -Shtd $@ > $(basename $@).lst
	$(RVPREFIX)-objcopy -O binary $@ $(basename $@).bin
	@printf "$(CLR_BL)$(CLR_B)  Binary size:$(CLR_NB) %s bytes$(CLR_NC) ($(basename $@).bin)\n" $$(stat --format=%s "$(basename $@).bin")

-include $(OUT)/deps.d

.PHONY: run
run: $(ELF)									#t# Run the example in atomsim
	$(call print_msg,Running, $<)
	@test -x $(ATOMSIM) || { printf "$(CLR_RD)$(CLR_B)ERROR:$(CLR_NC) atomsim not built at $(ATOMSIM)\n"; exit 1; }
	$(ATOMSIM) $(ATOMSIM_ARGS) $<

.PHONY: load
load: $(ELF)								#t# Upload to a board in UART boot mode (port=..., or a simulator)
	$(call print_msg,Loading, $(basename $<).bin)
	atomterm --bin $(basename $<).bin $(port)

.PHONY: dism
dism: $(ELF)								#t# Dump disassembly
	$(call print_msg,Generating disassembly,$(basename $<).lst)
	$(RVPREFIX)-objdump -htd $< > $(basename $<).lst
endif

.PHONY: clean
clean:										#t# Clean build outputs
	$(call print_msg,Cleaning,$(OUT))
	rm -rf $(OUT)
