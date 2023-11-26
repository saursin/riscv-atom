////////////////////////////////////////////////////////////////////   
//  File        : HydrogeSoC_Config.vh
//  Author      : Saurabh Singh (saurabh.s99100@gmail.com)
//  Description : Configuration file for HydrogenSoC. It defines the 
//      HydrogensoC related parameters using verilog preprocessor macros.
////////////////////////////////////////////////////////////////////
`ifndef __HYDROGENSOC_CONFIG_VH__
`define __HYDROGENSOC_CONFIG_VH__

////////////////////////////// SoC Config Macros //////////////////////////////
/*
    Reset address for SOC. Upon Reset, PC jumps to the defined address
*/
`ifndef SOC_RESET_ADDRESS
`define SOC_RESET_ADDRESS   32'h0001_0000
`endif


////////////////////////////// SoC Memory Map //////////////////////////////
// Macros used by Wishbone interconnect
// see: <link to memory map description>

// ROM
`define BOOTROM_ADDR    32'h0001_0000
`define BOOTROM_SIZE    8192            // 8 KB

// RAM
`define RAM_ADDR        32'h2000_0000
`define RAM_SIZE        49152           // 48 KB

// UART
`define UART_ADDR       32'h4000_0000
`define UART_SIZE       16              // 16 Bytes  (4 words)

// GPIO
`define GPIO_ADDR       32'h4000_2000
`define GPIO_SIZE       12              // 12 bytes (3 words)

// SPI
`define SPI_ADDR        32'h4000_3000
`define SPI_SIZE        32              // 32 bytes (8 words)

// TIMER
`define TIMER_ADDR      32'h4000_4000
`define TIMER_SIZE      16              // 32 bytes (4 words)


`endif // __HYDROGENSOC_CONFIG_VH__
