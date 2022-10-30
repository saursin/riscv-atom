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

// Enable E ext.
// `define RV_E

// Enable ZICSR ext.
`define RV_ZICSR

// Enable C extenstion
// `define RV_C


`define NGPIO 16    // Number of GPIO Pins to implement


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
`define UART_SIZE       8               // 8 Bytes  (2 words)

// GPIO
`define GPIO_ADDR       32'h4000_2000
`define GPIO_SIZE       16              // 16 bytes (4 words)


`endif // __HYDROGENSOC_CONFIG_VH__
