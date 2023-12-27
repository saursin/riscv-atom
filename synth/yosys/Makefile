#h# ***** Makefile to run Yosys synthesis *****

#v# Specify soctarget
soctarget?=hydrogensoc

#v# Specify build directory
build_dir?=build

# ----- Config - Common -----
log_file?=$(build_dir)/synth.log
build_info_file=$(build_dir)/build.info
sources_tcl_script=$(build_dir)/sources.tcl

# ----- Config - FPGA -----
#v# Specify FPGA Vendor
fpga_vendor?=xilinx

#v# Specify FPGA Family
fpga_family?=xc6s

fpga_synth_tcl_script?=synth_fpga.tcl

# ----- Config - ASIC -----
#v# Specify ASIC PDK
asic_pdk?=freepdk45

asic_synth_tcl_script?=synth_asic.tcl
asic_pdk_file:=$(build_dir)/$(asic_pdk)_stdcells.lib

_mk_check_env=1
include ../../common.mk

# Flags to the makefiles
MKFLAGS := -s 

$(shell mkdir -p $(build_dir))

default: fpga										#t# alias for fpga target

$(build_dir)/bootloader.hex:
	$(call print_msg_root,Rebuilding libcatom and bootloader)
	make $(MKFLAGS) -C $(RVATOM)/sw/lib soctarget=$(soctarget) sim=0 clean build
	make $(MKFLAGS) -C $(RVATOM)/sw/bootloader soctarget=$(soctarget) sim=0 clean build
	cp $(RVATOM)/sw/bootloader/bootloader.hex $@

$(build_dir)/HydrogenSoC.v:
	$(call print_msgt_root,Generating verilog)
	verilator -E -P `cfgparse.py $(RVATOM)/rtl/config/$(soctarget).json -T verilator -f` -DSOC_BOOTROM_INIT_FILE='"bootloader.hex"' > $@
	verilator --lint-only $@ -top-module `cfgparse.py $(RVATOM)/rtl/config/$(soctarget).json -T verilator -t`
	
	$(call print_msg_root,Dumping buildinfo,$(build_info_file))
	@echo "timestamp: `date +\"%m-%d-%Y %H:%M:%S\"`" > $(build_info_file);
	@echo "commit:    `git rev-parse HEAD`" >> $(build_info_file);
	@echo "soctarget: $(soctarget)" >> $(build_info_file);
	@echo "RTL:       $@" >> $(build_info_file);	



$(asic_pdk_file):
	$(call print_msgt_root,Downloading PDK)
ifeq ($(asic_pdk), freepdk45)
	wget https://raw.githubusercontent.com/mflowgen/freepdk-45nm/master/stdcells.lib -O $(asic_pdk_file)
else
	$(error "Invalid pdk: $(asic_pdk)")
endif

#########################################
# FPGA
.PHONY: fpga
fpga: $(build_dir)/bootloader.hex $(build_dir)/HydrogenSoC.v				#t# run fpga synthesis
	@echo "fpga:      $(fpga_vendor)::$(fpga_family)" >> $(build_info_file);	

	$(call print_msg_root,Running Yosys synth: FPGA)
	yosys -p 'tcl $(fpga_synth_tcl_script) $(fpga_vendor) $(fpga_family) $(build_dir)' 2>&1 > $(log_file) 


#########################################
# ASIC
.PHONY: asic
asic: $(build_dir)/bootloader.hex $(asic_pdk_file) $(build_dir)/HydrogenSoC.v	#t# run asic synthesis
	@echo "liberty:   $(asic_pdk)" >> $(build_info_file)

	$(call print_msg_root,Running Yosys synth: ASIC)
	yosys -p 'tcl $(asic_synth_tcl_script) $(asic_pdk_file) $(build_dir)' 2>&1 > $(log_file) 


.PHONY: clean
clean:												#t# clean build dir
	$(call print_msg_root,Cleaning build files)
	rm -rf $(build_dir)