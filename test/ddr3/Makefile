# cocotb testbenches for DDR3 on the PHY+DRAM model rtl/uncore/ddr/ddr3_phy_model.sv:
#   TB=ctrl  the vendored controller (rtl/uncore/ddr/gowin-ddr3) on its own
#   TB=wb    DDR3_wb: Wishbone adapter, refresh timer and registers around it
#
#   make                         # both
#   make TB=wb [TESTCASE=...]    # one testbench [one test]
#
# Requires cocotb (see $RVATOM/.venv) and Verilator.

RTL := $(abspath ../../rtl)

TOPLEVEL_LANG   = verilog
SIM            ?= verilator
VERILOG_SOURCES = $(abspath tb_ddr3_$(TB).sv)
TOPLEVEL        = tb_ddr3_$(TB)
MODULE          = test_ddr3_$(TB)
SIM_BUILD       = sim_build_$(TB)
COMPILE_ARGS   += -I$(RTL)/uncore/ddr/gowin-ddr3 -I$(RTL)/uncore/ddr -I$(RTL)/common

ifeq ($(TB),)
.DEFAULT_GOAL := all
.PHONY: all
all:
	$(MAKE) TB=ctrl
	$(MAKE) TB=wb
else
.DEFAULT_GOAL := sim
include $(shell cocotb-config --makefiles)/Makefile.sim
endif

clean::
	rm -rf sim_build_* results.xml __pycache__
