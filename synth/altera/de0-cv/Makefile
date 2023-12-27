#h# ***** Makefile to run FPGA flow *****

#v# Specify soctarget
soctarget?=hydrogensoc

#v# Specify build directory
build_dir?=build

_mk_check_env=1
include ../../../common.mk

# Flags to the makefiles
MKFLAGS := -s 

$(shell mkdir -p $(build_dir))

default: build

$(build_dir)/bootloader.hex:
	$(call print_msg_root,Rebuilding libcatom and bootloader)
	make $(MKFLAGS) -C $(RVATOM)/sw/lib soctarget=$(soctarget) sim=0 clean build
	make $(MKFLAGS) -C $(RVATOM)/sw/bootloader soctarget=$(soctarget) sim=0 clean build
	cp $(RVATOM)/sw/bootloader/bootloader.hex $@

$(build_dir)/HydrogenSoC.v:
	$(call print_msgt_root,Generating verilog)
	verilator -E -P `cfgparse.py $(RVATOM)/rtl/config/$(soctarget).json -T verilator -f` -DSOC_BOOTROM_INIT_FILE='"bootloader.hex"' -DSOC_INVERT_RST > $@
	verilator --lint-only $@ -top-module `cfgparse.py $(RVATOM)/rtl/config/$(soctarget).json -T verilator -t`

build: $(build_dir)/bootloader.hex $(build_dir)/HydrogenSoC.v	#t# Build FPGA Bitstream
	$(call print_msg_root,Starting FPGA Flow)
	cp -f HydrogenSoC.tcl $(build_dir)/
	verilator -E -P `cfgparse.py $(RVATOM)/rtl/config/$(soctarget).json -d` HydrogenSoC.sdc > $(build_dir)/HydrogenSoC.sdc
	cd $(build_dir) && time quartus_sh -t HydrogenSoC.tcl 2>&1 > build.log

.PHONY: prog
prog: 				#t#	Program FPGA
	$(call print_msg_root,Programming FPGA,$(build_dir)/output_files/HydrogenSoC.sof)
	quartus_pgm -m jtag -o "p;$(build_dir)/output_files/HydrogenSoC.sof"

.PHONY: clean
clean:				#t# Clean build files
	$(call print_msg_root,Cleaning build files)
	rm -rf $(build_dir)/*
	