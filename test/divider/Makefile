# cocotb testbench for rtl/lib/Divider.v
#
#   make                         # WIDTH=32 SKIP_W=8: edge cases + random
#   make WIDTH=8 SKIP_W=2        # small width: exhaustive
#   make all-configs             # both of the above
#
# Requires cocotb (see $RVATOM/.venv) and Verilator.

WIDTH  ?= 32
SKIP_W ?= 8
export WIDTH SKIP_W

TOPLEVEL_LANG   = verilog
SIM            ?= verilator
VERILOG_SOURCES = $(abspath ../../rtl/lib/Divider.v)
TOPLEVEL        = Divider
MODULE          = test_divider
SIM_BUILD       = sim_build_w$(WIDTH)_s$(SKIP_W)
COMPILE_ARGS   += -GWIDTH=$(WIDTH) -GSKIP_W=$(SKIP_W)

.DEFAULT_GOAL := sim

.PHONY: all-configs
all-configs:
	$(MAKE) WIDTH=8 SKIP_W=2
	$(MAKE) WIDTH=32 SKIP_W=8

include $(shell cocotb-config --makefiles)/Makefile.sim

clean::
	rm -rf sim_build_* results.xml __pycache__
