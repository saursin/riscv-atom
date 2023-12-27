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
	@printf "$(CLR_GR)>> Generating verilog$(CLR_NC)\n"
	verilator -E -P `cfgparse.py $(RVATOM)/rtl/config/$(soctarget).json -T verilator -f` -DSOC_BOOTROM_INIT_FILE='"bootloader.hex"' > $@
	verilator --lint-only $@ -top-module `cfgparse.py $(RVATOM)/rtl/config/$(soctarget).json -T verilator -t`

build: $(build_dir)/bootloader.hex $(build_dir)/HydrogenSoC.v		#t# Build FPGA Bitstream
	$(call print_msg_root,Starting FPGA Flow)
	cp -f spartan6-mini.tcl $(build_dir)/
	verilator -E -P `cfgparse.py $(RVATOM)/rtl/config/$(soctarget).json -d` HydrogenSoC.ucf > $(build_dir)/HydrogenSoC.ucf
	cd $(build_dir) && time xtclsh spartan6-mini.tcl rebuild_project 2>&1 > build.log

.PHONY: prog
prog: 				#t#	Program FPGA
	$(call print_msg_root,Programming FPGA,$(build_dir)/HydrogenSoC.bit)
	openFPGALoader --fpga-part xc6slx9tqg144 -f --verify $(build_dir)/HydrogenSoC.bit -r

.PHONY: clean
clean:				#t# Clean build files
	$(call print_msg_root,Cleaning build files)
	rm -rf $(build_dir)/*
	