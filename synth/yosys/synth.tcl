set rtl_dir                 ../../rtl
set rd_vrlg_flags           "-DSYNTHESIS_YOSYS -D__ROM_INIT_FILE__=\"init.hex\" -I ../../rtl -I ../../rtl/common -I ../../rtl/core -DRV_E"
set verilog_topmodule       HydrogenSoC

# Verilog Files
yosys "read_verilog $rd_vrlg_flags $rtl_dir/HydrogenSoC_Config.vh"
yosys "read_verilog $rd_vrlg_flags $rtl_dir/HydrogenSoC.v"
yosys "read_verilog $rd_vrlg_flags $rtl_dir/core/AtomRV.v"
yosys "read_verilog $rd_vrlg_flags $rtl_dir/core/Alu.v"
yosys "read_verilog $rd_vrlg_flags $rtl_dir/core/Decode.v "
yosys "read_verilog $rd_vrlg_flags $rtl_dir/core/RegisterFile.v "
yosys "read_verilog $rd_vrlg_flags $rtl_dir/core/CSR_Unit.v"
yosys "read_verilog $rd_vrlg_flags $rtl_dir/core/AtomRV_wb.v "
yosys "read_verilog $rd_vrlg_flags $rtl_dir/uncore/wishbone/arbiter.v"
yosys "read_verilog $rd_vrlg_flags $rtl_dir/uncore/wishbone/arbiter3_wb.v"
yosys "read_verilog $rd_vrlg_flags $rtl_dir/uncore/wishbone/priority_encoder.v"
yosys "read_verilog $rd_vrlg_flags $rtl_dir/uncore/gpio/IOBuf.v"
yosys "read_verilog $rd_vrlg_flags $rtl_dir/uncore/gpio/GPIO.v"
yosys "read_verilog $rd_vrlg_flags $rtl_dir/uncore/mem/DualPortRAM_wb.v"
yosys "read_verilog $rd_vrlg_flags $rtl_dir/uncore/mem/SinglePortRAM_wb.v"
yosys "read_verilog $rd_vrlg_flags $rtl_dir/uncore/uart/UART.v"
yosys "read_verilog $rd_vrlg_flags $rtl_dir/uncore/uart/UART_core.v"

# Heirarchy check
yosys "hierarchy -check -top $verilog_topmodule"

# Synthesize
yosys "synth_xilinx -family xc6s"
# yosys "synth_xilinx -family xc7"

# Report Utilization
yosys "tee -a report.txt stat"