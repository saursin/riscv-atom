#h# ***** Makefile to run FPGA flow *****

#v# Specify soctarget
soctarget?=hydrogensoc

#v# Board config (rtl/config/<soctarget>-<board>.yml)
board?=de0-cv

#v# Specify build directory
build_dir?=build

# keep synth lib/bootloader out of the sim workspace
RVATOM_BUILD ?= $(abspath $(build_dir))

_mk_check_env=1
_mk_check_soctarget=1
include ../../../common.mk

# Flags to the makefiles
MKFLAGS := -s 

$(shell mkdir -p $(build_dir))

# board shell (pads, reset) around the SoC
VTOPMODULE := de0cv_shell
BOARD_VSRCS := de0cv_shell.v $(RVATOM)/rtl/uncore/gpio/IOBank.v $(RVATOM)/rtl/lib/IOBuf.v $(RVATOM)/rtl/lib/reset_sync.v

default: build

$(build_dir)/bootloader.hex: $(RVATOM_VARS_MK)
	$(call print_msg_root,Rebuilding libcatom and bootloader)
	$(MAKE) $(MKFLAGS) -C $(RVATOM)/sw/lib soctarget=$(soctarget) board=$(board) RVATOM_BUILD=$(RVATOM_BUILD) clean build
	$(MAKE) $(MKFLAGS) -C $(RVATOM)/sw/bootloader soctarget=$(soctarget) board=$(board) RVATOM_BUILD=$(RVATOM_BUILD) clean build
	cp $(RVATOM_BUILD)/sw/bootloader/bootloader.hex $@

$(build_dir)/HydrogenSoC.v: $(RVATOM_VARS_MK)
	$(call print_msgt_root,Generating verilog)
	verilator -E -P `$(CFGPARSE) -c $(RVATOM_YMLCONFIG) -t vflags` -DSOC_BOOTROM_INIT_FILE='"bootloader.hex"' $(BOARD_VSRCS) > $@
	verilator --lint-only $@ -top-module $(VTOPMODULE)

build: $(build_dir)/bootloader.hex $(build_dir)/HydrogenSoC.v	#t# Build FPGA Bitstream
	$(call print_msg_root,Starting FPGA Flow)
	cp -f HydrogenSoC.tcl $(build_dir)/
	verilator -E -P $(RVATOM_VDEFINES) HydrogenSoC.sdc > $(build_dir)/HydrogenSoC.sdc
	cd $(build_dir) && time quartus_sh -t HydrogenSoC.tcl 2>&1 > build.log

.PHONY: prog
prog: 				#t#	Program FPGA
	$(call print_msg_root,Programming FPGA,$(build_dir)/output_files/HydrogenSoC.sof)
	quartus_pgm -m jtag -o "p;$(build_dir)/output_files/HydrogenSoC.sof"

.PHONY: clean
clean:				#t# Clean build files
	$(call print_msg_root,Cleaning build files)
	rm -rf $(build_dir)/*
	