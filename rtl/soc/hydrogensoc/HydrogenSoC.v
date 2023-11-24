///////////////////////////////////////////////////////////////////
//  File        : HydrogenSoC.v
//  Author      : Saurabh Singh (saurabh.s99100@gmail.com)
//  Description : HydrogenSoC is an FPGA ready SoC, it consists of
//      a single atom core with memories and communication modules.
///////////////////////////////////////////////////////////////////
`include "HydrogenSoC_Config.vh"
`include "Utils.vh"

`ifdef verilator
    // Macros for Verilator
`endif
`ifdef SYNTHESIS
    // Macros for Yosys & Xilinx ISE
    `ifndef __ROM_INIT_FILE__
    `define __ROM_INIT_FILE__ "rom.hex"
    `endif
`endif

`ifndef __ROM_INIT_FILE__
`define __ROM_INIT_FILE__ ""
`endif

// converts memory aperture size to 32 bit mask for wishbone crossbar
`define size_to_mask32(sz) (-32'h1 << $clog2(sz))

`default_nettype none

module HydrogenSoC(
    // GLOBAL SIGNALS
    input   wire                clk_i,
    input   wire                rst_i,

    // GPIO
    inout   wire [`NGPIO-1:0]   gpio_io,

    // UART
    input   wire                uart_mux_sel_i,
    input   wire                uart_usb_rx_i,
    output  wire                uart_usb_tx_o,
    input   wire                uart_io_rx_i,
    output  wire                uart_io_tx_o,

    // SPI
    input   wire                spi_miso_i,
    output  wire                spi_mosi_o,
    output  wire                spi_sck_o,
    output  wire [`NSPI_CS-1:0] spi_cs_o
);
    wire wb_clk_i = clk_i;
    wire wb_rst_i = rst_i;

    // ******************** FrontPort ********************
    // FrontPort Signals
    `ifdef SOC_FRONTPORT
    wire    [31:0]  fp_wb_adr_o     = 32'h00000000;
    wire    [31:0]  fp_wb_dat_o     = 32'h00000000;
    wire    [31:0]  fp_wb_dat_i;    `UNUSED_VAR(fp_wb_dat_i)
    wire            fp_wb_we_o      = 1'b0;
    wire    [3:0]   fp_wb_sel_o     = 4'b0000;
    wire            fp_wb_stb_o     = 1'b0;
    wire            fp_wb_ack_i;    `UNUSED_VAR(fp_wb_ack_i)
    wire            fp_wb_cyc_o     = 1'b0;
    `endif


    // ******************** Core ********************
    wire    [31:0]  core_iport_wb_adr_o;
    wire    [31:0]  core_iport_wb_dat_o     = 32'h000000;
    wire    [31:0]  core_iport_wb_dat_i;
    wire            core_iport_wb_we_o      = 1'b0;
    wire    [3:0]   core_iport_wb_sel_o     = 4'b1111;
    wire            core_iport_wb_stb_o;
    wire            core_iport_wb_ack_i;
    wire            core_iport_wb_cyc_o;
    
    wire    [31:0]  core_dport_wb_adr_o;
    wire    [31:0]  core_dport_wb_dat_o;
    wire    [31:0]  core_dport_wb_dat_i;
    wire            core_dport_wb_we_o;
    wire    [3:0]   core_dport_wb_sel_o;
    wire            core_dport_wb_stb_o;
    wire            core_dport_wb_ack_i;
    wire            core_dport_wb_cyc_o;

    AtomRV_wb atom_wb_core (   
        .wb_clk_i        (wb_clk_i),
        .wb_rst_i        (wb_rst_i),
        
        .reset_vector_i  (`SOC_RESET_ADDRESS),
        
        .iport_wb_adr_o  (core_iport_wb_adr_o),
        .iport_wb_dat_i  (core_iport_wb_dat_i),
        .iport_wb_cyc_o  (core_iport_wb_cyc_o),
        .iport_wb_stb_o  (core_iport_wb_stb_o),
        .iport_wb_ack_i  (core_iport_wb_ack_i),
        
        .dport_wb_adr_o  (core_dport_wb_adr_o),
        .dport_wb_dat_o  (core_dport_wb_dat_o),
        .dport_wb_dat_i  (core_dport_wb_dat_i),
        .dport_wb_we_o   (core_dport_wb_we_o),
        .dport_wb_sel_o  (core_dport_wb_sel_o),
        .dport_wb_stb_o  (core_dport_wb_stb_o),
        .dport_wb_ack_i  (core_dport_wb_ack_i),
        .dport_wb_cyc_o  (core_dport_wb_cyc_o)

        `ifdef EN_EXCEPT
        ,
        .irq_i          (1'b0),
        .timer_int_i    (timer_int_o)
        `endif // EN_EXCEPT
    );


    // ******************** Arbiter ********************
    wire    [31:0]  arb_wb_adr_o;
    wire    [31:0]  arb_wb_dat_i;
    wire    [31:0]  arb_wb_dat_o;
    wire            arb_wb_we_o;
    wire    [3:0]   arb_wb_sel_o;
    wire            arb_wb_stb_o;
    wire            arb_wb_ack_i;
    wire            arb_wb_cyc_o;
    wire            arb_wb_err_i;
    `UNUSED_VAR(arb_wb_err_i)

    `ifdef SOC_FRONTPORT
    Arbiter3_wb #(
    `else
    Arbiter2_wb #(
    `endif
        .DATA_WIDTH (32),
        .ADDR_WIDTH (32),
        .SELECT_WIDTH (4)
    ) arbiter (
        .clk        (clk_i),
        .rst        (rst_i),

        // Wishbone master 0 input
        .wbm0_adr_i     (core_iport_wb_adr_o),
        .wbm0_dat_i     (core_iport_wb_dat_o),
        .wbm0_dat_o     (core_iport_wb_dat_i),
        .wbm0_we_i      (core_iport_wb_we_o),
        .wbm0_sel_i     (core_iport_wb_sel_o),
        .wbm0_stb_i     (core_iport_wb_stb_o),
        .wbm0_ack_o     (core_iport_wb_ack_i),
        .wbm0_cyc_i     (core_iport_wb_cyc_o),

        // Wishbone master 1 input
        .wbm1_adr_i     (core_dport_wb_adr_o),
        .wbm1_dat_i     (core_dport_wb_dat_o),
        .wbm1_dat_o     (core_dport_wb_dat_i),
        .wbm1_we_i      (core_dport_wb_we_o),
        .wbm1_sel_i     (core_dport_wb_sel_o),
        .wbm1_stb_i     (core_dport_wb_stb_o),
        .wbm1_ack_o     (core_dport_wb_ack_i),
        .wbm1_cyc_i     (core_dport_wb_cyc_o),

        `ifdef SOC_FRONTPORT
        // Wishbone master 2 input
        .wbm2_adr_i     (fp_wb_adr_o),
        .wbm2_dat_i     (fp_wb_dat_o),
        .wbm2_dat_o     (fp_wb_dat_i),
        .wbm2_we_i      (fp_wb_we_o),
        .wbm2_sel_i     (fp_wb_sel_o),
        .wbm2_stb_i     (fp_wb_stb_o),
        .wbm2_ack_o     (fp_wb_ack_i),
        .wbm2_cyc_i     (fp_wb_cyc_o),
        `endif

        // Wishbone slave output
        .wbs_adr_o      (arb_wb_adr_o),
        .wbs_dat_i      (arb_wb_dat_i),
        .wbs_dat_o      (arb_wb_dat_o),
        .wbs_we_o       (arb_wb_we_o), 
        .wbs_sel_o      (arb_wb_sel_o),
        .wbs_stb_o      (arb_wb_stb_o),
        .wbs_ack_i      (arb_wb_ack_i),
        .wbs_cyc_o      (arb_wb_cyc_o)
    );


    // ******************** Crossbar ********************
    Crossbar6_wb #(
        .DATA_WIDTH     (32),
        .ADDR_WIDTH     (32),
        .DEVICE0_ADDR   (`BOOTROM_ADDR),
        .DEVICE0_MASK   (`size_to_mask32(`BOOTROM_SIZE)),
        .DEVICE1_ADDR   (`RAM_ADDR),
        .DEVICE1_MASK   (`size_to_mask32(`RAM_SIZE)),
        .DEVICE2_ADDR   (`UART_ADDR),
        .DEVICE2_MASK   (`size_to_mask32(`UART_SIZE)),
        .DEVICE3_ADDR   (`GPIO_ADDR),
        .DEVICE3_MASK   (`size_to_mask32(`GPIO_SIZE)),
        .DEVICE4_ADDR   (`SPI_ADDR),
        .DEVICE4_MASK   (`size_to_mask32(`SPI_SIZE)),
        .DEVICE5_ADDR   (`TIMER_ADDR),
        .DEVICE5_MASK   (`size_to_mask32(`TIMER_SIZE))
    ) xbar (
        .wbs_adr_i      (arb_wb_adr_o),
        .wbs_dat_i      (arb_wb_dat_o),
        .wbs_dat_o      (arb_wb_dat_i),
        .wbs_we_i       (arb_wb_we_o),
        .wbs_sel_i      (arb_wb_sel_o),
        .wbs_stb_i      (arb_wb_stb_o),
        .wbs_cyc_i      (arb_wb_cyc_o),
        .wbs_ack_o      (arb_wb_ack_i),
        .wbs_err_o      (arb_wb_err_i),

        .wbm0_adr_o     (bootrom_wb_adr_i),
        .wbm0_dat_i     (bootrom_wb_dat_o),
        /* verilator lint_off PINCONNECTEMPTY */
        .wbm0_dat_o     (),
        .wbm0_we_o      (),
        .wbm0_sel_o     (),
        /* verilator lint_on PINCONNECTEMPTY */
        .wbm0_cyc_o     (bootrom_wb_cyc_i),
        .wbm0_stb_o     (bootrom_wb_stb_i),
        .wbm0_ack_i     (bootrom_wb_ack_o),
        .wbm0_err_i     (1'b0),

        .wbm1_adr_o     (ram_wb_adr_i),
        .wbm1_dat_i     (ram_wb_dat_o),
        .wbm1_dat_o     (ram_wb_dat_i),
        .wbm1_we_o      (ram_wb_we_i),
        .wbm1_sel_o     (ram_wb_sel_i),
        .wbm1_cyc_o     (ram_wb_cyc_i),
        .wbm1_stb_o     (ram_wb_stb_i),
        .wbm1_ack_i     (ram_wb_ack_o),
        .wbm1_err_i     (1'b0),

        .wbm2_adr_o     (uart_wb_adr_i),
        .wbm2_dat_i     (uart_wb_dat_o),
        .wbm2_dat_o     (uart_wb_dat_i),
        .wbm2_we_o      (uart_wb_we_i),
        .wbm2_sel_o     (uart_wb_sel_i),
        .wbm2_cyc_o     (uart_wb_cyc_i),
        .wbm2_stb_o     (uart_wb_stb_i),
        .wbm2_ack_i     (uart_wb_ack_o),
        .wbm2_err_i     (1'b0),

        .wbm3_adr_o     (gpio_wb_adr_i),
        .wbm3_dat_i     (gpio_wb_dat_o),
        .wbm3_dat_o     (gpio_wb_dat_i),
        .wbm3_we_o      (gpio_wb_we_i),
        .wbm3_sel_o     (gpio_wb_sel_i),
        .wbm3_cyc_o     (gpio_wb_cyc_i),
        .wbm3_stb_o     (gpio_wb_stb_i),
        .wbm3_ack_i     (gpio_wb_ack_o),
        .wbm3_err_i     (1'b0),

        .wbm4_adr_o     (spi_wb_adr_i),
        .wbm4_dat_i     (spi_wb_dat_o),
        .wbm4_dat_o     (spi_wb_dat_i),
        .wbm4_we_o      (spi_wb_we_i),
        .wbm4_sel_o     (spi_wb_sel_i),
        .wbm4_cyc_o     (spi_wb_cyc_i),
        .wbm4_stb_o     (spi_wb_stb_i),
        .wbm4_ack_i     (spi_wb_ack_o),
        .wbm4_err_i     (1'b0),

        .wbm5_adr_o     (timer_wb_adr_i),
        .wbm5_dat_i     (timer_wb_dat_o),
        .wbm5_dat_o     (timer_wb_dat_i),
        .wbm5_we_o      (timer_wb_we_i),
        .wbm5_sel_o     (timer_wb_sel_i),
        .wbm5_cyc_o     (timer_wb_cyc_i),
        .wbm5_stb_o     (timer_wb_stb_i),
        .wbm5_ack_i     (timer_wb_ack_o),
        .wbm5_err_i     (1'b0)
    );


    // ******************** BOOTROM ********************
    parameter BOOTROM_ADR_SIZE = $clog2(`BOOTROM_SIZE);

    /* verilator lint_off UNUSEDSIGNAL */
    wire [31:0] bootrom_wb_adr_i;
    /* verilator lint_on UNUSEDSIGNAL */
    wire [31:0] bootrom_wb_dat_o;
    wire        bootrom_wb_stb_i;
    wire        bootrom_wb_cyc_i;
    wire        bootrom_wb_ack_o;
    
    SinglePortROM_wb #(
        .ADDR_WIDTH(BOOTROM_ADR_SIZE),
        .MEM_FILE(`__ROM_INIT_FILE__)
    ) bootrom (
        .wb_clk_i   (wb_clk_i),
        .wb_rst_i   (wb_rst_i),

        .wb_adr_i   (bootrom_wb_adr_i[BOOTROM_ADR_SIZE-1:2]),
        .wb_dat_o   (bootrom_wb_dat_o),
        .wb_stb_i   (bootrom_wb_stb_i & bootrom_wb_cyc_i),
        .wb_ack_o   (bootrom_wb_ack_o)
    );


    // ******************** RAM ********************
    parameter RAM_ADR_SIZE = $clog2(`RAM_SIZE);

    /* verilator lint_off UNUSEDSIGNAL */
    wire  [31:0]    ram_wb_adr_i;
    /* verilator lint_on UNUSEDSIGNAL */
    wire  [31:0]    ram_wb_dat_o;
    wire  [31:0]    ram_wb_dat_i;
    wire 		    ram_wb_we_i;
    wire  [3:0]     ram_wb_sel_i;
    wire            ram_wb_cyc_i;
    wire            ram_wb_stb_i;
    wire 		    ram_wb_ack_o;

    SinglePortRAM_wb #(
        .ADDR_WIDTH(RAM_ADR_SIZE),
        .MEM_FILE()
    ) ram (
        .wb_clk_i   (wb_clk_i),
        .wb_rst_i   (wb_rst_i),

        .wb_adr_i   (ram_wb_adr_i[RAM_ADR_SIZE-1:2]),
        .wb_dat_o   (ram_wb_dat_o),
        .wb_dat_i   (ram_wb_dat_i),
        .wb_we_i    (ram_wb_we_i),
        .wb_sel_i   (ram_wb_sel_i),
        .wb_stb_i   (ram_wb_stb_i & ram_wb_cyc_i),
        .wb_ack_o   (ram_wb_ack_o)
    );


    // ******************** UART ********************
    /* verilator lint_off UNUSEDSIGNAL */
    wire  [31:0]    uart_wb_adr_i;
    /* verilator lint_on UNUSEDSIGNAL */
    wire  [31:0]    uart_wb_dat_o;
    wire  [31:0]    uart_wb_dat_i;
    wire 		    uart_wb_we_i;
    wire  [3:0]     uart_wb_sel_i;
    wire            uart_wb_cyc_i;
    wire            uart_wb_stb_i;
    wire 		    uart_wb_ack_o;

    // Uart Mux Logic
    wire    uart_rx_i = uart_mux_sel_i ? uart_usb_rx_i : uart_io_rx_i;
    wire    uart_tx_o;
    assign  uart_io_tx_o = uart_mux_sel_i ? uart_tx_o : 1'b1;
    assign  uart_usb_tx_o = uart_mux_sel_i ? 1'b1 : uart_tx_o;
    
    UART uart (
        .wb_clk_i   (wb_clk_i),
        .wb_rst_i   (wb_rst_i),

        .wb_adr_i   (uart_wb_adr_i[3:2]),
        .wb_dat_o   (uart_wb_dat_o),
        .wb_dat_i   (uart_wb_dat_i),
        .wb_we_i    (uart_wb_we_i),
        .wb_sel_i   (uart_wb_sel_i),
        .wb_stb_i   (uart_wb_stb_i & uart_wb_cyc_i),
        .wb_ack_o   (uart_wb_ack_o),

        .rx_i       (uart_rx_i),
        .tx_o       (uart_tx_o)
    );


    // ******************** GPIO ********************
    /* verilator lint_off UNUSEDSIGNAL */
    wire  [31:0]    gpio_wb_adr_i;
    /* verilator lint_on UNUSEDSIGNAL */
    wire  [31:0]    gpio_wb_dat_o;
    wire  [31:0]    gpio_wb_dat_i;
    wire 		    gpio_wb_we_i;
    wire  [3:0]     gpio_wb_sel_i;
    wire            gpio_wb_cyc_i;
    wire            gpio_wb_stb_i;
    wire 		    gpio_wb_ack_o;
    
    GPIO #(
        .N(`NGPIO)
    ) gpio (
        .wb_clk_i   (wb_clk_i),
        .wb_rst_i   (wb_rst_i),

        .wb_adr_i   (gpio_wb_adr_i[3:2]),
        .wb_dat_o   (gpio_wb_dat_o),
        .wb_dat_i   (gpio_wb_dat_i),
        .wb_we_i    (gpio_wb_we_i),
        .wb_sel_i   (gpio_wb_sel_i),
        .wb_stb_i   (gpio_wb_stb_i & gpio_wb_cyc_i),
        .wb_ack_o   (gpio_wb_ack_o),

        .gpio_io    (gpio_io)
    );


    // ******************** SPI ********************
    /* verilator lint_off UNUSEDSIGNAL */
    wire  [31:0]    spi_wb_adr_i;
    /* verilator lint_on UNUSEDSIGNAL */
    wire  [31:0]    spi_wb_dat_o;
    wire  [31:0]    spi_wb_dat_i;
    wire 		    spi_wb_we_i;
    wire  [3:0]     spi_wb_sel_i;
    wire            spi_wb_cyc_i;
    wire            spi_wb_stb_i;
    wire 		    spi_wb_ack_o;
    SPI_wb #(
        .NCS(`NSPI_CS)
    ) spi (
        .wb_clk_i   (wb_clk_i),
        .wb_rst_i   (wb_rst_i),

        .wb_adr_i   (spi_wb_adr_i[4:2]),
        .wb_dat_o   (spi_wb_dat_o),
        .wb_dat_i   (spi_wb_dat_i),
        .wb_we_i    (spi_wb_we_i),
        .wb_sel_i   (spi_wb_sel_i),
        .wb_stb_i   (spi_wb_stb_i & spi_wb_cyc_i),
        .wb_ack_o   (spi_wb_ack_o),

        .sck_o      (spi_sck_o),
        .miso_i     (spi_miso_i),
        .mosi_o     (spi_mosi_o),
        .cs_o       (spi_cs_o)
    );

    // ******************* TIMER *******************
    /* verilator lint_off UNUSEDSIGNAL */
    wire  [31:0]    timer_wb_adr_i;
    /* verilator lint_on UNUSEDSIGNAL */
    wire  [31:0]    timer_wb_dat_o;
    wire  [31:0]    timer_wb_dat_i;
    wire 		    timer_wb_we_i;
    wire  [3:0]     timer_wb_sel_i;
    wire            timer_wb_cyc_i;
    wire            timer_wb_stb_i;
    wire 		    timer_wb_ack_o;
    wire            timer_int_o;

    Timer_wb timer (
        .wb_clk_i   (wb_clk_i),
        .wb_rst_i   (wb_rst_i),

        .wb_adr_i   (timer_wb_adr_i[3:2]),
        .wb_dat_o   (timer_wb_dat_o),
        .wb_dat_i   (timer_wb_dat_i),
        .wb_we_i    (timer_wb_we_i),
        .wb_sel_i   (timer_wb_sel_i),
        .wb_stb_i   (timer_wb_stb_i & timer_wb_cyc_i),
        .wb_ack_o   (timer_wb_ack_o),

        .int_o      (timer_int_o)
);


endmodule
