`ifndef __ATOMBONES_CONFIG_VH__
`define __ATOMBONES_CONFIG_VH__

// Reset address
`define SOC_RESET_ADDRESS   32'h0001_0000

`define RV_ZICSR

`define RV_C

`define EN_EXCEPT

// Atombones doesn't need the following parameters since everything 
// other that the core is software simulated

// SoC Peripherals

// IRAM
// `define IRAM_ADDR   32'h00000000    // 0 GB boundry
// `define IRAM_SIZE   32'h00008000    // 32 KB

// // RAM
// `define RAM_ADDR    32'h04000000    // 0.5 GB boundry
// `define RAM_SIZE    32'h00002000    // 8 KB

// // UART
// `define UART_ADDR   32'h08000000    // 1.0 GB boundry
// `define UART_SIZE   32'h00000008    // 8 bytes

`endif //__ATOMBONES_CONFIG_VH__
