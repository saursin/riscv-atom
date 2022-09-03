////////////////////////////////////////////////////////////////////   
//  File        : HydrogeSoC_Config.vh
//  Author      : Saurabh Singh (saurabh.s99100@gmail.com)
//  Description : Configuration file for HydrogenSoC. It defines the 
//      HydrogensoC related parameters using verilog preprocessor macros.
////////////////////////////////////////////////////////////////////

`ifndef __HYDROGENSOC_CONFIG_VH__
`define __HYDROGENSOC_CONFIG_VH__

// Reset address
`define SOC_RESET_ADDRESS   32'h0001_0000


/////////////// Memory Map ///////////////
// Macros used by Wishbone interconnect
// see: <link to memory map description>

// ROM
`define ROM_ADDR        32'h0001_0000
`define ROM_SIZE        32'h0000_4000   // 16 KB

// RAM
`define RAM_ADDR        32'h2000_0000
`define RAM_SIZE        32'h0000_8000   // 32 KB

// UART
`define UART_ADDR       32'h4000_0000
`define UART_SIZE       32'h0000_0008   // 8 bytes

// GPIO 0
`define GPIO0_ADDR      32'h4002_0000
`define GPIO0_SIZE      32'h0000_0004   // 4 bytes

// GPIO 1
`define GPIO1_ADDR      32'h4002_0100
`define GPIO1_SIZE      32'h0000_0004   // 4 bytes


`endif // __HYDROGENSOC_CONFIG_VH__
