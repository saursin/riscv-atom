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
`define SOC_RESET_ADDRESS   32'h0001_0000

/*
    If defined, adds a front port to the SoC which can master the System bus. 
    The frontport has the highest priority among all the ports. User can connect 
    external master devices to the frontport.
*/
`define SOC_FRONTPORT


/*
    If defined, enables the RVE extenstion which disables the upper 16 registers 
    in the register file.
*/
// `define RV_E


/*
    If defined, enables the RISC-V Zicsr extension which adds support for CSR 
    registers. It adds a CSR Unit to the Core which implements CSR registers 
    like CYCLEL/H.
*/
`define RV_ZICSR


/*
    If defined, enables the RISC-V Compressed extension which adds support for 16-bit 
    compressed instructions. It adds a decoder which decodes the 16 bit instructions 
    to 32 bit equivalents and a aligner to word-align Instruction fetches.
*/
// `define RV_C


/*
    Define number of GPIO Pins to implement
*/
`define NGPIO 16




////////////////////////////// SoC Memory Map //////////////////////////////
// Macros used by Wishbone interconnect
// see: <link to memory map description>

// ROM
`define ROM_ADDR        32'h0001_0000
`define ROM_SIZE        32'h0000_8000   // 32 KB

// RAM
`define RAM_ADDR        32'h2000_0000
`define RAM_SIZE        32'h0000_4000   // 16 KB

// UART
`define UART_ADDR       32'h4000_0000
`define UART_SIZE       16              // 16 Bytes  (4 words)

// GPIO
`define GPIO_ADDR       32'h4000_2000
`define GPIO_SIZE       12              // 12 bytes (3 words)


`endif // __HYDROGENSOC_CONFIG_VH__
