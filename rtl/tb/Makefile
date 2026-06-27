include ../../common.mk

YMLCONFIG := $(RVATOM)/rtl/config/hydrogensoc.yml

default: trace.vcd

trace.vcd: out.vvp
	vvp out.vvp

out.vvp: filelist.f HydrogenSoC_tb.v
	iverilog -o $@ -s isim_tb -f filelist.f HydrogenSoC_tb.v

filelist.f:
	$(CFGPARSE) -c $(YMLCONFIG) -t filelist -o $@

.PHONY: clean
clean:
	rm -f *.vvp *.vcd *.f