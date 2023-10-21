if {[llength $argv] != 2} {
    puts "Usage: script.tcl <filelist.F> <reportfile>"
    exit 1
}

set listfile [lindex $argv 0]
set reportfile [lindex $argv 1]

# Parse Verilog list file
source ../utils.tcl
set listfile_contents [parse_listfile $listfile]
set vfiles [lindex $listfile_contents 0]
set incdirs [lindex $listfile_contents 1]

# read verilog files
set rd_vrlg_flags "-DSYNTHESIS_YOSYS -D__ROM_INIT_FILE__=\"init.hex\""
foreach dir $incdirs {
    append rd_vrlg_flags " -I$dir"
}

foreach file $vfiles {
    yosys "read_verilog $rd_vrlg_flags $file"
}

# Heirarchy check
yosys "hierarchy -check"

# Synthesize
# set fpga_family xc7
set fpga_family xc6s
yosys "synth_xilinx -family $fpga_family"

# Report Utilization
yosys "tee -a $reportfile stat"
