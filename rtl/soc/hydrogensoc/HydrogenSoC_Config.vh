///////////////////////////////////////////////////////////////////////////////   
//  File        : HydrogeSoC_Config.vh
//  Author      : Saurabh Singh (saurabh.s99100@gmail.com)
//  Description : Configuration file for HydrogenSoC. It defines the 
//      HydrogensoC related parameters using verilog preprocessor macros.
///////////////////////////////////////////////////////////////////////////////
`ifndef __HYDROGENSOC_CONFIG_VH__
`define __HYDROGENSOC_CONFIG_VH__

`ifdef verilator
    // Macros for Verilator
`endif

`ifdef SYNTHESIS
    // Macros for Synthesis tools like Yosys & Xilinx ISE
    `ifndef SOC_BOOTROM_INIT_FILE
    `define SOC_BOOTROM_INIT_FILE "rom.hex"
    `endif
`endif


////////////////////////////// SoC Config Macros //////////////////////////////
/*
    The following macros are used to enable/disable features/peripherals and set 
    parameters. To enable a peripheral, uncomment the respective 
    `define SOC_EN_XYZ macro or define the same on CLI while invoking the tool
*/


/*
    -------------------------------------------------------
    BootROM: Initialized with bootloader hex code
*/
`define SOC_BOOTROM_ADDR        32'h0001_0000
`define SOC_BOOTROM_SIZE        (8*1024)

`ifndef SOC_BOOTROM_INIT_FILE
    `define SOC_BOOTROM_INIT_FILE ""
`endif


/*
    -------------------------------------------------------
    RAM
*/
`define SOC_RAM_ADDR            32'h2000_0000
`define SOC_RAM_SIZE            (48*1024)


/*
    -------------------------------------------------------
    UART Peripheral (Optional)
*/
// `define SOC_EN_UART
`define SOC_UART_ADDR           32'h4000_0000
`define SOC_UART_SIZE           16              // 4 words
`define SOC_UART_FIFO_EN        1
`define SOC_UART_FIFO_DEPTH     4


/*
    -------------------------------------------------------
    GPIO Peripheral (Optional)
*/
// `define SOC_EN_GPIO
`define SOC_GPIO_ADDR           32'h4000_2000
`define SOC_GPIO_SIZE           12              // 3 words

`ifndef SOC_GPIO_NUM_PINS
    `define SOC_GPIO_NUM_PINS   32
`endif


/*
    -------------------------------------------------------
    SPI Peripheral (Optional)
*/
// `define SOC_EN_SPI
`define SOC_SPI_ADDR            32'h4000_3000
`define SOC_SPI_SIZE            32              // 8 words

`ifndef SOC_SPI_NUM_CS
    `define SOC_SPI_NUM_CS      1
`endif


/*
    -------------------------------------------------------
    TIMER Peripheral (Optional)
*/
// `define SOC_EN_TIMER
`define SOC_TIMER_ADDR          32'h4000_4000
`define SOC_TIMER_SIZE          16              // 4 words


/*
    -------------------------------------------------------
    Reset address for SOC. Upon Reset, PC jumps to the defined address
*/
`ifndef SOC_RESET_ADDRESS
`define SOC_RESET_ADDRESS       `SOC_BOOTROM_ADDR
`endif

///////////////////////////////////////////////////////////////////////////////
// Helper macros

// converts memory aperture size to 32 bit mask for wishbone crossbar
`define size_to_mask32(sz) (-32'h1 << $clog2(sz))

`define IS_DEFINED(X) `ifdef X 1 `else 0 `endif

// Counts the number of peripherals to be connected to crossbar's master interface
`define SOC_XBAR_SLAVE_COUNT ( 2 + `IS_DEFINED(SOC_EN_UART) + `IS_DEFINED(SOC_EN_GPIO) + `IS_DEFINED(SOC_EN_SPI) + `IS_DEFINED(SOC_EN_TIMER))

`endif // __HYDROGENSOC_CONFIG_VH__
