default: trace.vcd

trace.vcd: out.vvp
	vvp out.vvp

out.vvp: 
	iverilog -o $@ `$(RVATOM)/scripts/cfgparse.py $(RVATOM)/rtl/config/hydrogensoc.json -f --tool=iverilog` HydrogenSoC_tb.v -s isim_tb

.PHONY: clean
clean:
	rm -f *.vvp *.vcd