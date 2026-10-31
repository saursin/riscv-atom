#h# ***** Makefile to run FPGA flow *****

#v# Specify soctarget
soctarget?=hydrogensoc

#v# Board config (rtl/config/<soctarget>-<board>.yml)
board?=tangprimer20k

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
VTOPMODULE := tangprimer20k_shell
BOARD_VSRCS := tangprimer20k_shell.v tangprimer20k_pll.v $(RVATOM)/rtl/uncore/gpio/IOBank.v $(RVATOM)/rtl/lib/IOBuf.v $(RVATOM)/rtl/lib/reset_sync.v
BOARD_VSRCS += $(RVATOM)/rtl/uncore/ddr/gowin-ddr3/ddr3/ddr3_phy_slow.sv	# DDR3 PHY (SystemVerilog)

default: build

$(build_dir)/bootloader.hex: $(RVATOM_VARS_MK)
	$(call print_msg_root,Rebuilding libcatom and bootloader)
	$(MAKE) $(MKFLAGS) -C $(RVATOM)/sw/lib soctarget=$(soctarget) board=$(board) RVATOM_BUILD=$(RVATOM_BUILD) clean build
	$(MAKE) $(MKFLAGS) -C $(RVATOM)/sw/bootloader soctarget=$(soctarget) board=$(board) RVATOM_BUILD=$(RVATOM_BUILD) clean build
	cp $(RVATOM_BUILD)/sw/bootloader/bootloader.hex $@

$(build_dir)/HydrogenSoC.v: $(RVATOM_VDEPS) $(BOARD_VSRCS) $(RVATOM_VARS_MK)
	$(call print_msgt_root,Generating verilog)
	verilator -E -P $(RVATOM_VFLAGS) -DSOC_BOOTROM_INIT_FILE='"bootloader.hex"' $(BOARD_VSRCS) > $@
	verilator --lint-only $@ $(addprefix $(shell yosys-config --datdir)/gowin/,cells_sim.v cells_xtra_gw2a.v) -top-module $(VTOPMODULE)

# Timing constraints from the board config: the PLL output must match clk_freq
$(build_dir)/$(VTOPMODULE).sdc: tangprimer20k_pll.v $(RVATOM_VARS_MK)
	@pll=$$(sed -n 's/.*Achieved output frequency: *\([0-9.]*\) MHz.*/\1/p' tangprimer20k_pll.v); \
	awk -v p="$$pll" -v f=$(RVATOM_CLK_FREQ) 'BEGIN { exit !(p * 1000000 == f) }' \
		|| { echo "ERROR: tangprimer20k_pll.v gives $$pll MHz, config clk_freq is $(RVATOM_CLK_FREQ) Hz (regenerate it with gowin_pll)"; exit 1; }
	printf 'create_clock -name clk_27mhz_i -period 37.037 [get_ports {clk_27mhz_i}]\n' > $@
	awk -v f=$(RVATOM_CLK_FREQ) 'BEGIN { printf "create_clock -name clk -period %.3f [get_nets {clk}]\n", 1e9 / f }' >> $@

.PHONY: build
build: $(build_dir)/bootloader.hex $(build_dir)/HydrogenSoC.v $(build_dir)/$(VTOPMODULE).sdc	#t# Build FPGA Bitstream
	$(call print_msg_root,Starting FPGA Flow)
	bash run.sh --build-dir=$(build_dir) --steps=synth,pnr,pack

################################################################################
# Programming flow
.PHONY: prog
prog: $(build_dir)/$(VTOPMODULE)_pack.fs	#t# Program FPGA
	$(call print_msg_root,Programming FPGA)
	bash run.sh --build-dir=$(build_dir) --steps=prog


# .PHONY: prog
# prog: 				#t#	Program FPGA
# 	$(call print_msg_root,Programming FPGA,$(build_dir)/output_files/HydrogenSoC.sof)
# quartus_pgm -m jtag -o "p;$(build_dir)/output_files/HydrogenSoC.sof"

.PHONY: clean
clean:				#t# Clean build files
	$(call print_msg_root,Cleaning build files)
	rm -rf $(build_dir)/*
	