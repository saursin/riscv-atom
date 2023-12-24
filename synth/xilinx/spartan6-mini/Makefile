soctarget?=hydrogensoc
build_dir?=build

include ../../../common.mk

$(shell mkdir -p $(build_dir))

default: build

$(build_dir)/bootloader.hex:
	@printf "$(CLR_GR)>> Rebuilding libcatom and bootloader$(CLR_NC)\n"
	make -C $(RVATOM)/sw/lib soctarget=$(soctarget) sim=0 clean default
	make -C $(RVATOM)/sw/bootloader soctarget=$(soctarget) sim=0 clean build
	cp $(RVATOM)/sw/bootloader/bootloader.hex $@

$(build_dir)/HydrogenSoC.v:
	@printf "$(CLR_GR)>> Generating verilog$(CLR_NC)\n"
	verilator -E -P `cfgparse.py $(RVATOM)/rtl/config/$(soctarget).json -T verilator -f` -DSOC_BOOTROM_INIT_FILE='"bootloader.hex"' > $@
	verilator --lint-only $@ -top-module `cfgparse.py $(RVATOM)/rtl/config/$(soctarget).json -T verilator -t`

build: $(build_dir)/bootloader.hex $(build_dir)/HydrogenSoC.v
	@printf "$(CLR_GR)>> Starting FPGA build$(CLR_NC)\n"
	cp -f spartan6-mini.tcl $(build_dir)/
	verilator -E -P `cfgparse.py $(RVATOM)/rtl/config/$(soctarget).json -d` HydrogenSoC.ucf > $(build_dir)/HydrogenSoC.ucf
	cd $(build_dir) && time xtclsh spartan6-mini.tcl rebuild_project 2>&1 > build.log

.PHONY: clean
clean:
	@printf "$(CLR_GR)>> Cleaning build files$(CLR_NC)\n"
	rm -rf $(build_dir)/*
	