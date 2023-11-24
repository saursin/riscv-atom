include ../../common.mk

#########################################
# Config - Common
soctarget?=hydrogensoc
build_dir?=build
log_file?=$(build_dir)/synth.log
build_info_file=$(build_dir)/build.info
sources_tcl_script=$(build_dir)/sources.tcl

# Config - FPGA
fpga_vendor?=xilinx
fpga_family?=xc6s
fpga_synth_tcl_script?=synth_fpga.tcl

# Config - ASIC
asic_pdk?=freepdk45
asic_synth_tcl_script?=synth_asic.tcl
#########################################
$(shell mkdir -p $(build_dir))
asic_pdk_file:=$(build_dir)/$(asic_pdk)_stdcells.lib

default: fpga										## alias for fpga target

$(build_dir)/init.hex:
	make -C $(RVATOM) soctarget=$(soctarget) boot
	cp $(RVATOM)/sw/bootloader/bootloader.hex $@

$(sources_tcl_script):
	@printf "$(CLR_GR)>> Generating sources: $(sources_tcl_script)$(CLR_NC)\n"
	$(RVATOM)/scripts/cfgparse.py $(RVATOM)/rtl/config/$(soctarget).json --tcl -o $(sources_tcl_script)
	
	@printf "$(CLR_GR)>> Dumping build info: $(build_info_file)$(CLR_NC)\n"
	@echo "timestamp: `date +\"%m-%d-%Y %H:%M:%S\"`" > $(build_info_file);
	@echo "commit:    `git rev-parse HEAD`" >> $(build_info_file);
	@echo "soctarget: $(soctarget)" >> $(build_info_file);


$(asic_pdk_file):
	@printf "$(CLR_GR)>> Downloading PDK: $(asic_pdk) $(CLR_NC)\n"
ifeq ($(asic_pdk), freepdk45)
	wget https://raw.githubusercontent.com/mflowgen/freepdk-45nm/master/stdcells.lib -O $(asic_pdk_file)
else
$(error "Invalid pdk: $(asic_pdk)")
endif

#########################################
# FPGA
.PHONY: fpga
fpga: $(build_dir)/init.hex $(sources_tcl_script)					## run fpga synthesis
	@echo "script:    $(fpga_synth_tcl_script)" >> $(build_info_file);	
	@echo "fpga:      $(fpga_vendor)::$(fpga_family)" >> $(build_info_file);	

	@printf "$(CLR_GR)>> Running Yosys: FPGA$(CLR_NC)\n"
	yosys -p 'tcl $(fpga_synth_tcl_script) $(sources_tcl_script) $(fpga_vendor) $(fpga_family) $(build_dir)' 2>&1 > $(log_file) 


#########################################
# ASIC
.PHONY: asic
asic: $(build_dir)/init.hex $(asic_pdk_file) $(sources_tcl_script)	## run asic synthesis
	@echo "script:    $(asic_synth_tcl_script)" >> $(build_info_file)
	@echo "liberty:   $(asic_pdk)" >> $(build_info_file)

	@printf "$(CLR_GR)>> Running Yosys: ASIC$(CLR_NC)\n"
	yosys -p 'tcl $(asic_synth_tcl_script) $(sources_tcl_script) $(asic_pdk_file) $(build_dir)' 2>&1 > $(log_file) 


.PHONY: clean
clean:												## clean build dir
	@printf "$(CLR_GR)>> Cleaning build dir $(CLR_NC)\n"
	rm -rf $(build_dir)

	
.PHONY : help
help : Makefile					## show help
	@echo "Usage:"
	@echo "	$$ make [TARGET]"
	@echo ""
	@echo "TARGETs:"
	@grep -E -h '\s##\s' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\t\033[36m%-20s\033[0m %s\n", $$1, $$2}'
	