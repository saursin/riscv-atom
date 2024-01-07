# Copyright (C) 1991-2013 Altera Corporation
# Your use of Altera Corporation's design tools, logic functions 
# and other software and tools, and its AMPP partner logic 
# functions, and any output files from any of the foregoing 
# (including device programming or simulation files), and any 
# associated documentation or information are expressly subject 
# to the terms and conditions of the Altera Program License 
# Subscription Agreement, Altera MegaCore Function License 
# Agreement, or other applicable license agreement, including, 
# without limitation, that your use is for the sole purpose of 
# programming logic devices manufactured by Altera and sold by 
# Altera or its authorized distributors.  Please refer to the 
# applicable agreement for further details.

# Quartus II: Generate Tcl File for Project
# File: HydrogenSoC.tcl
# Generated on: Sun Dec 24 19:31:41 2023

# Load Quartus II Tcl Project package
package require ::quartus::project

set need_to_close_project 0
set make_assignments 1

# Check that the right project is open
if {[is_project_open]} {
	if {[string compare $quartus(project) "HydrogenSoC"]} {
		puts "Project HydrogenSoC is not open"
		set make_assignments 0
	}
} else {
	# Only open if not already open
	if {[project_exists HydrogenSoC]} {
		project_open -revision HydrogenSoC HydrogenSoC
	} else {
		project_new -revision HydrogenSoC HydrogenSoC
	}
	set need_to_close_project 1
}

# Make assignments
if {$make_assignments} {
	set_global_assignment -name FAMILY "Cyclone V"
	set_global_assignment -name DEVICE 5CEBA4F23C7
	set_global_assignment -name ORIGINAL_QUARTUS_VERSION "13.0 SP1"
	set_global_assignment -name PROJECT_CREATION_TIME_DATE "17:27:32  DECEMBER 24, 2023"
	set_global_assignment -name LAST_QUARTUS_VERSION "13.0 SP1"
	set_global_assignment -name PROJECT_OUTPUT_DIRECTORY output_files
	set_global_assignment -name MIN_CORE_JUNCTION_TEMP 0
	set_global_assignment -name MAX_CORE_JUNCTION_TEMP 85
	set_global_assignment -name ERROR_CHECK_FREQUENCY_DIVISOR 256
	set_global_assignment -name POWER_PRESET_COOLING_SOLUTION "23 MM HEAT SINK WITH 200 LFPM AIRFLOW"
	set_global_assignment -name POWER_BOARD_THERMAL_MODEL "NONE (CONSERVATIVE)"
	set_global_assignment -name PARTITION_NETLIST_TYPE SOURCE -section_id Top
	set_global_assignment -name PARTITION_FITTER_PRESERVATION_LEVEL PLACEMENT_AND_ROUTING -section_id Top
	set_global_assignment -name PARTITION_COLOR 16764057 -section_id Top
	set_global_assignment -name STRATIX_DEVICE_IO_STANDARD "2.5 V"
	set_global_assignment -name SDC_FILE HydrogenSoC.sdc
	set_global_assignment -name VERILOG_FILE HydrogenSoC.v

	###########################################################################
	# clock and reset
	set_location_assignment PIN_M9 -to clk_i
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to clk_i
	set_location_assignment PIN_U7 -to rst_i
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to rst_i

	###########################################################################
	# UART
	set_location_assignment PIN_T17 -to uart_rx_i
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to uart_rx_i
	set_location_assignment PIN_T15 -to uart_tx_o
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to uart_tx_o

	###########################################################################
	# GPIO - LED
	set_location_assignment PIN_AA2 -to gpio_io[0]
	set_location_assignment PIN_AA1 -to gpio_io[1]
	set_location_assignment PIN_W2 -to gpio_io[2]
	set_location_assignment PIN_Y3 -to gpio_io[3]
	set_location_assignment PIN_N2 -to gpio_io[4]
	set_location_assignment PIN_N1 -to gpio_io[5]
	set_location_assignment PIN_U2 -to gpio_io[6]
	set_location_assignment PIN_U1 -to gpio_io[7]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to gpio_io[0]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to gpio_io[1]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to gpio_io[2]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to gpio_io[3]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to gpio_io[4]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to gpio_io[5]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to gpio_io[6]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to gpio_io[7]
	set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to gpio_io[0]
	set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to gpio_io[1]
	set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to gpio_io[2]
	set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to gpio_io[3]
	set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to gpio_io[4]
	set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to gpio_io[5]
	set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to gpio_io[6]
	set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to gpio_io[7]
	set_instance_assignment -name SLEW_RATE 1 -to gpio_io[0]
	set_instance_assignment -name SLEW_RATE 1 -to gpio_io[1]
	set_instance_assignment -name SLEW_RATE 1 -to gpio_io[2]
	set_instance_assignment -name SLEW_RATE 1 -to gpio_io[3]
	set_instance_assignment -name SLEW_RATE 1 -to gpio_io[4]
	set_instance_assignment -name SLEW_RATE 1 -to gpio_io[5]
	set_instance_assignment -name SLEW_RATE 1 -to gpio_io[6]
	set_instance_assignment -name SLEW_RATE 1 -to gpio_io[7]

	###########################################################################
	# GPIO - Pushbuttons
	set_location_assignment PIN_U13 -to gpio_io[8]
	set_location_assignment PIN_V13 -to gpio_io[9]
	set_location_assignment PIN_T13 -to gpio_io[10]
	set_location_assignment PIN_T12 -to gpio_io[11]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to gpio_io[8]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to gpio_io[9]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to gpio_io[10]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to gpio_io[11]

	###########################################################################
	# GPIO - 7 Seg
	set_location_assignment PIN_U21 -to gpio_io[12]
	set_location_assignment PIN_V21 -to gpio_io[13]
	set_location_assignment PIN_W22 -to gpio_io[14]
	set_location_assignment PIN_W21 -to gpio_io[15]
	set_location_assignment PIN_Y22 -to gpio_io[16]
	set_location_assignment PIN_Y21 -to gpio_io[17]
	set_location_assignment PIN_AA22 -to gpio_io[18]
	set_location_assignment PIN_AA20 -to gpio_io[19]
	set_location_assignment PIN_AB20 -to gpio_io[20]
	set_location_assignment PIN_AA19 -to gpio_io[21]
	set_location_assignment PIN_AA18 -to gpio_io[22]
	set_location_assignment PIN_AB18 -to gpio_io[23]
	set_location_assignment PIN_AA17 -to gpio_io[24]
	set_location_assignment PIN_U22 -to gpio_io[25]
	set_location_assignment PIN_Y19 -to gpio_io[26]
	set_location_assignment PIN_AB17 -to gpio_io[27]
	set_location_assignment PIN_AA10 -to gpio_io[28]
	set_location_assignment PIN_Y14 -to gpio_io[29]
	set_location_assignment PIN_V14 -to gpio_io[30]
	set_location_assignment PIN_AB22 -to gpio_io[31]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to gpio_io[12]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to gpio_io[13]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to gpio_io[14]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to gpio_io[15]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to gpio_io[16]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to gpio_io[17]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to gpio_io[18]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to gpio_io[19]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to gpio_io[20]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to gpio_io[21]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to gpio_io[22]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to gpio_io[23]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to gpio_io[24]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to gpio_io[25]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to gpio_io[26]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to gpio_io[27]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to gpio_io[28]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to gpio_io[29]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to gpio_io[30]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to gpio_io[31]

	# SDCARD Slot
	set_location_assignment PIN_C11 -to spi_cs_o[0]
	set_location_assignment PIN_H11 -to spi_sck_o 
	set_location_assignment PIN_B11 -to spi_mosi_o
	set_location_assignment PIN_K9 	-to spi_miso_i
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to spi_cs_o[0]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to spi_sck_o
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to spi_mosi_o
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to spi_miso_i

	# GPIO - SDCARD ELA
	# set_location_assignment PIN_G13 -to ela_spi_cs_o
	# set_location_assignment PIN_G12 -to ela_spi_sck_o
	# set_location_assignment PIN_J17 -to ela_spi_mosi_o
	# set_location_assignment PIN_K16 -to ela_spi_miso_o
	# set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to ela_spi_cs_o[0]
	# set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to ela_spi_sck_o
	# set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to ela_spi_mosi_o
	# set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to ela_spi_miso_o

	set_instance_assignment -name PARTITION_HIERARCHY root_partition -to | -section_id Top

	# Commit assignments
	export_assignments
}

# Load necessary package
load_package flow

# Synthesize the design
puts "Synthesizing the design..."
execute_module -tool map

# Place and route
puts "Performing place and route..."
execute_module -tool fit

# Generate programming files (Bitstream)
puts "Generating bitstream..."
execute_module -tool asm

# Generate Timing Analysis Report
puts "Generating timing analysis report..."
execute_module -tool sta

puts "Bitstream generation completed."

if {$need_to_close_project} {
		project_close
}