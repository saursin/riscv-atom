## Generated SDC file "r5p_soc_top.sdc"

## Copyright (C) 2021  Intel Corporation. All rights reserved.
## Your use of Intel Corporation's design tools, logic functions
## and other software and tools, and any partner logic
## functions, and any output files from any of the foregoing
## (including device programming or simulation files), and any
## associated documentation or information are expressly subject
## to the terms and conditions of the Intel Program License
## Subscription Agreement, the Intel Quartus Prime License Agreement,
## the Intel FPGA IP License Agreement, or other applicable license
## agreement, including, without limitation, that your use is for
## the sole purpose of programming logic devices manufactured by
## Intel and sold by Intel or its authorized distributors.  Please
## refer to the applicable agreement for further details, at
## https://fpgasoftware.intel.com/eula.


## VENDOR  "Altera"
## PROGRAM "Quartus Prime"
## VERSION "Version 21.1.0 Build 842 10/21/2021 SJ Lite Edition"

## DATE    "Fri Feb 11 20:53:01 2022"

##
## DEVICE  "5CEBA4F23C7"
##


#**************************************************************
# Time Information
#**************************************************************

set_time_format -unit ns -decimal_places 3

#**************************************************************
# Create Clock
#**************************************************************

#create_clock -name {clk_i} -period  20.000 -waveform {0.000 10.000} [get_ports {clk_i}]  # 50MHz

#**************************************************************
# Create Generated Clock
#**************************************************************

#create_clock -name {CLOCK_SYS} -period 100.000 -waveform {0.000 50.000} [get_ports {clk_i}];  #  10.0 MHz
#create_clock -name {CLOCK_SYS} -period  40.000 -waveform {0.000 20.000} [get_ports {clk_i}];  #  25.0 MHz
#create_clock -name {CLOCK_SYS} -period  30.000 -waveform {0.000 15.000} [get_ports {clk_i}];  #  33.3 MHz
#create_clock -name {CLOCK_SYS} -period  25.000 -waveform {0.000 12.500} [get_ports {clk_i}];  #  40.0 MHz (with NO extra adders)

create_clock -name {CLOCK_SYS} -period  20.000 -waveform {0.000 10.000} [get_ports {clk_i}];  #  50.0 MHz

#create_clock -name {CLOCK_SYS} -period  18.000 -waveform {0.000  9.000} [get_ports {clk_i}];  #  55.5 MHz
#create_clock -name {CLOCK_SYS} -period  16.000 -waveform {0.000  8.000} [get_ports {clk_i}];  #  62.5 MHz
#create_clock -name {CLOCK_SYS} -period  15.000 -waveform {0.000  7.500} [get_ports {clk_i}];  #  66.6 MHz (with ALL extra adders)
#create_clock -name {CLOCK_SYS} -period  12.500 -waveform {0.000  6.250} [get_ports {clk_i}];  #  80.0 MHz
#create_clock -name {CLOCK_SYS} -period  12.000 -waveform {0.000  6.000} [get_ports {clk_i}];  #  83.3 MHz
#create_clock -name {CLOCK_SYS} -period  10.000 -waveform {0.000  5.000} [get_ports {clk_i}];  # 100.0 MHz

#**************************************************************
# Set Clock Latency
#**************************************************************



#**************************************************************
# Set Clock Uncertainty
#**************************************************************

set_clock_uncertainty -rise_from [get_clocks {CLOCK_SYS}] -rise_to [get_clocks {CLOCK_SYS}] -setup 0.100;
set_clock_uncertainty -rise_from [get_clocks {CLOCK_SYS}] -rise_to [get_clocks {CLOCK_SYS}] -hold  0.060;
set_clock_uncertainty -rise_from [get_clocks {CLOCK_SYS}] -fall_to [get_clocks {CLOCK_SYS}] -setup 0.100;
set_clock_uncertainty -rise_from [get_clocks {CLOCK_SYS}] -fall_to [get_clocks {CLOCK_SYS}] -hold  0.060;
set_clock_uncertainty -fall_from [get_clocks {CLOCK_SYS}] -rise_to [get_clocks {CLOCK_SYS}] -setup 0.100;
set_clock_uncertainty -fall_from [get_clocks {CLOCK_SYS}] -rise_to [get_clocks {CLOCK_SYS}] -hold  0.060;
set_clock_uncertainty -fall_from [get_clocks {CLOCK_SYS}] -fall_to [get_clocks {CLOCK_SYS}] -setup 0.100;
set_clock_uncertainty -fall_from [get_clocks {CLOCK_SYS}] -fall_to [get_clocks {CLOCK_SYS}] -hold  0.060;

#**************************************************************
# Set Input Delay
#**************************************************************



#**************************************************************
# Set Output Delay
#**************************************************************



#**************************************************************
# Set Clock Groups
#**************************************************************



#**************************************************************
# Set False Path
#**************************************************************



#**************************************************************
# Set Multicycle Path
#**************************************************************



#**************************************************************
# Set Maximum Delay
#**************************************************************



#**************************************************************
# Set Minimum Delay
#**************************************************************



#**************************************************************
# Set Input Transition
#**************************************************************