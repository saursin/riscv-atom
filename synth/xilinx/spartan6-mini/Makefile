#h# ***** Makefile to run FPGA flow *****

#v# Specify soctarget
soctarget?=hydrogensoc

#v# Specify build directory
build_dir?=build

# keep synth (sim=0) lib/bootloader out of the sim workspace
RVATOM_BUILD ?= $(abspath $(build_dir))

_mk_check_env=1
_mk_check_soctarget=1
include ../../../common.mk

# Flags to the makefiles
MKFLAGS := -s 

$(shell mkdir -p $(build_dir))

VTOPMODULE := $(RVATOM_TOPMODULE)

default: build

$(build_dir)/bootloader.hex: $(RVATOM_VARS_MK)
	$(call print_msg_root,Rebuilding libcatom and bootloader)
	$(MAKE) $(MKFLAGS) -C $(RVATOM)/sw/lib soctarget=$(soctarget) RVATOM_BUILD=$(RVATOM_BUILD) sim=0 clean build
	$(MAKE) $(MKFLAGS) -C $(RVATOM)/sw/bootloader soctarget=$(soctarget) RVATOM_BUILD=$(RVATOM_BUILD) sim=0 clean build
	cp $(RVATOM_BUILD)/sw/bootloader/bootloader.hex $@

$(build_dir)/HydrogenSoC.v: $(RVATOM_VARS_MK)
	$(call print_msgt_root,Generating verilog)
	@printf "$(CLR_GR)>> Generating verilog$(CLR_NC)\n"
	verilator -E -P `$(CFGPARSE) -c $(RVATOM_YMLCONFIG) -t vflags` -DSOC_BOOTROM_INIT_FILE='"bootloader.hex"' > $@
	verilator --lint-only $@ -top-module $(VTOPMODULE)

build: $(build_dir)/bootloader.hex $(build_dir)/HydrogenSoC.v		#t# Build FPGA Bitstream
	$(call print_msg_root,Starting FPGA Flow)
	cp -f spartan6-mini.tcl $(build_dir)/
	verilator -E -P $(RVATOM_VDEFINES) HydrogenSoC.ucf > $(build_dir)/HydrogenSoC.ucf
	cd $(build_dir) && time xtclsh spartan6-mini.tcl rebuild_project 2>&1 > build.log

.PHONY: prog
prog: 				#t#	Program FPGA
	$(call print_msg_root,Programming FPGA,$(build_dir)/HydrogenSoC.bit)
	openFPGALoader --fpga-part xc6slx9tqg144 -f --verify $(build_dir)/HydrogenSoC.bit -r

.PHONY: clean
clean:				#t# Clean build files
	$(call print_msg_root,Cleaning build files)
	rm -rf $(build_dir)/*
	