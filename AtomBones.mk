# Makefile vaiables for atombones simulation

# Verilog
verilog_topmodule = AtomBones
verilog_topmodule_file = $(rtl_dir)/AtomBones.v
verilog_files = rtl/AtomBones.v rtl/Timescale.vh rtl/Config.vh rtl/core/AtomRV.v rtl/core/Alu.v rtl/core/Decode.v rtl/core/RegisterFile.v