###############################################################################
# Synthesis script for yosys synthesis (FPGA)
###############################################################################
set time_start [clock seconds]; puts "Start time: [clock format ${time_start} -gmt false]"

if {[llength $argv] != 3} {
    puts "Usage: script.tcl <fpga_vendor> <fpga_family> <build_dir>"
    exit 1
}

###############################################################################
# Settings
set fpga_vendor [lindex $argv 0]
set fpga_family [lindex $argv 1]
set flatten 0

# Output directories
set build_dir [lindex $argv 2]
set output_dir "$build_dir/output"
set report_dir "$build_dir/report"

set vsource $build_dir/HydrogenSoC.v
set topmodule HydrogenSoC
###############################################################################
file mkdir $output_dir
file mkdir $report_dir

yosys "echo on"

# -- Read Verilog ----------------------
set rd_vrlg_flags "-DSYNTHESIS_YOSYS -DSOC_BOOTROM_INIT_FILE=\"$build_dir/bootloader.hex\""
yosys "read_verilog $rd_vrlg_flags $vsource"

# Check hierarchy
yosys "hierarchy -check -top $topmodule"

# Flatten
if {$flatten} {
    yosys "flatten"
}

# check for problems
yosys "check -assert"

# -- Synth -----------------------------
yosys "synth_$fpga_vendor -family $fpga_family"

# check for problems
yosys "check -mapped"

# -- Write reports ---------------------
yosys "tee -a $report_dir/utilization.rpt stat"

# prints detected loop warning
# yosys "tee -a $report_dir/ltp.rpt ltp"

yosys "tee -a $report_dir/timing.rpt sta"

yosys "clean"

# -- Write output ----------------------
yosys "write_verilog -v -attr2comment $output_dir/${topmodule}.v"
yosys "write_blif $output_dir/${topmodule}.blif"
yosys "write_rtlil $output_dir/${topmodule}.rtlil"

###############################################################################
set time_end [clock seconds]; puts "End time: [clock format ${time_start} -gmt false]"
source ../utils.tcl
print_elapsed_time $time_start $time_end
puts "----- We're done here -----"