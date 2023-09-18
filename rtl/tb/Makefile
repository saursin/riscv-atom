rtl_dir = $(RVATOM)/rtl

VFLAGS = -Wall
VFLAGS += -I$(RVATOM)/rtl/soc/hydrogensoc -I$(RVATOM)/rtl/common -I$(RVATOM)/rtl/core
VFLAGS += -D__ROM_INIT_FILE__='"$(RVATOM)/sw/flashboot/rom.hex"'

VSRCS  = $(rtl_dir)/tb/HydrogenSoC_tb.v
VSRCS += $(rtl_dir)/soc/hydrogensoc/HydrogenSoC.v
VSRCS += $(rtl_dir)/core/AtomRV.v
VSRCS += $(rtl_dir)/core/Alu.v
VSRCS += $(rtl_dir)/core/Decode.v
VSRCS += $(rtl_dir)/core/RegisterFile.v
VSRCS += $(rtl_dir)/core/CSR_Unit.v
VSRCS += $(rtl_dir)/core/AtomRV_wb.v 
VSRCS += $(rtl_dir)/uncore/wishbone/Arbiter.v
VSRCS += $(rtl_dir)/uncore/wishbone/Arbiter3_wb.v
VSRCS += $(rtl_dir)/uncore/wishbone/Crossbar5_wb.v
VSRCS += $(rtl_dir)/uncore/wishbone/Priority_encoder.v
VSRCS += $(rtl_dir)/uncore/gpio/IOBuf.v
VSRCS += $(rtl_dir)/uncore/gpio/GPIO.v
VSRCS += $(rtl_dir)/uncore/mem/DualPortRAM_wb.v
VSRCS += $(rtl_dir)/uncore/mem/SinglePortROM_wb.v
VSRCS += $(rtl_dir)/uncore/mem/SinglePortRAM_wb.v
VSRCS += $(rtl_dir)/uncore/uart/UART.v
VSRCS += $(rtl_dir)/uncore/uart/UART_core.v
VSRCS += $(rtl_dir)/uncore/spi/SPI_wb.v
VSRCS += $(rtl_dir)/uncore/spi/SPI_core.v

default: trace.vcd

trace.vcd: out.vvp
	vvp out.vvp

out.vvp: $(VSRCS)
	iverilog $(VFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -f *.vvp *.vcd