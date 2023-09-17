set rtl_dir                 ../../rtl
set rd_vrlg_flags           "-DSYNTHESIS_YOSYS -D__ROM_INIT_FILE__=\"init.hex\" -I ../../rtl -I ../../rtl/common -I ../../rtl/core"
set verilog_topmodule       HydrogenSoC

# Verilog Files
yosys "read_verilog $rd_vrlg_flags $rtl_dir/soc/hydrogensoc/HydrogenSoC_Config.vh"
yosys "read_verilog $rd_vrlg_flags $rtl_dir/soc/hydrogensoc/HydrogenSoC.v"
yosys "read_verilog $rd_vrlg_flags $rtl_dir/core/AtomRV.v"
yosys "read_verilog $rd_vrlg_flags $rtl_dir/core/Alu.v"
yosys "read_verilog $rd_vrlg_flags $rtl_dir/core/Decode.v "
yosys "read_verilog $rd_vrlg_flags $rtl_dir/core/RegisterFile.v "
yosys "read_verilog $rd_vrlg_flags $rtl_dir/core/CSR_Unit.v"
yosys "read_verilog $rd_vrlg_flags $rtl_dir/core/AtomRV_wb.v "
yosys "read_verilog $rd_vrlg_flags $rtl_dir/uncore/wishbone/Arbiter.v"
yosys "read_verilog $rd_vrlg_flags $rtl_dir/uncore/wishbone/Arbiter3_wb.v"
yosys "read_verilog $rd_vrlg_flags $rtl_dir/uncore/wishbone/Crossbar5_wb.v"
yosys "read_verilog $rd_vrlg_flags $rtl_dir/uncore/wishbone/Priority_encoder.v"
yosys "read_verilog $rd_vrlg_flags $rtl_dir/uncore/gpio/IOBuf.v"
yosys "read_verilog $rd_vrlg_flags $rtl_dir/uncore/gpio/GPIO.v"
yosys "read_verilog $rd_vrlg_flags $rtl_dir/uncore/mem/DualPortRAM_wb.v"
yosys "read_verilog $rd_vrlg_flags $rtl_dir/uncore/mem/SinglePortROM_wb.v"
yosys "read_verilog $rd_vrlg_flags $rtl_dir/uncore/mem/SinglePortRAM_wb.v"
yosys "read_verilog $rd_vrlg_flags $rtl_dir/uncore/uart/UART.v"
yosys "read_verilog $rd_vrlg_flags $rtl_dir/uncore/uart/UART_core.v"
yosys "read_verilog $rd_vrlg_flags $rtl_dir/uncore/spi/SPI_wb.v"
yosys "read_verilog $rd_vrlg_flags $rtl_dir/uncore/spi/SPI_core.v"

# Heirarchy check
yosys "hierarchy -check -top $verilog_topmodule"

# Synthesize
yosys "synth_xilinx -family xc6s"
# yosys "synth_xilinx -family xc7"

# Report Utilization
yosys "tee -a synth_report.txt stat"