`ifndef __HYDROGENSOC_CONFIG_VH__
`define __HYDROGENSOC_CONFIG_VH__

// Reset address
`define SOC_RESET_ADDRESS   32'h00000000

// SoC Peripherals

// IRAM
`define IRAM_ADDR   32'h00000000    // 0 GB boundry
`define IRAM_SIZE   32'h00008000    // 32 KB

// RAM
`define RAM_ADDR    32'h04000000    // 0.5 GB boundry
`define RAM_SIZE    32'h00002000    // 8 KB

// UART
`define UART_ADDR   32'h08000000    // 1.0 GB boundry
`define UART_SIZE   32'h00000008    // 8 bytes

// GPIO 0
`define GPIO0_ADDR  32'h08000100
`define GPIO0_SIZE  32'h00000004    // 4 bytes

// GPIO 1
`define GPIO1_ADDR  32'h08000104
`define GPIO1_SIZE  32'h00000004    // 4 bytes


`endif // __HYDROGENSOC_CONFIG_VH__
