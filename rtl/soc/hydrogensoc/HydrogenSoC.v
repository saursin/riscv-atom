///////////////////////////////////////////////////////////////////
//  File        : HydrogenSoC.v
//  Author      : Saurabh Singh (saurabh.s99100@gmail.com)
//  Description : HydrogenSoC is an FPGA ready SoC, it consists of
//      a single atom core with memories and communication modules.
///////////////////////////////////////////////////////////////////
`include "Utils.vh"
`include "HydrogenSoC_Config.vh"

`default_nettype none

module HydrogenSoC(
    input   wire    clk_i,
    input   wire    rst_i

`ifdef SOC_EN_UART
    ,
    input   wire    uart_rx_i,
    output  wire    uart_tx_o
`endif

`ifdef SOC_EN_GPIO
    ,
    inout   wire [`SOC_GPIO_NUM_PINS-1:0]   gpio_io
`endif

`ifdef SOC_EN_SPI
    ,
    input   wire                            spi_miso_i,
    output  wire                            spi_mosi_o,
    output  wire                            spi_sck_o,
    output  wire [`SOC_SPI_NUM_CS-1:0]      spi_cs_o
`endif

`ifdef EN_DEBUG
    ,
    input   wire                            jtag_tck_i,
    input   wire                            jtag_trst_n_i,
    input   wire                            jtag_tms_i,
    input   wire                            jtag_tdi_i,
    output  wire                            jtag_tdo_o
`endif
);
    wire wb_clk_i = clk_i;
    wire wb_rst_i = `INLINE_IFDEF(SOC_INVERT_RST, ~rst_i, rst_i);

    // ************* Debug Subsystem ****************
    `ifdef EN_DEBUG

    debug_subsystem debug_subsys (
        .clk_i          (wb_clk_i),
        .rst_i          (wb_rst_i),
        .jtag_tck_i     (jtag_tck_i),
        .jtag_trst_n_i  (jtag_trst_n_i),
        .jtag_tms_i     (jtag_tms_i),
        .jtag_tdi_i     (jtag_tdi_i),
        .jtag_tdo_o     (jtag_tdo_o)
    );

    `endif // EN_DEBUG

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
        .timer_int_i    (`INLINE_IFDEF(SOC_EN_TIMER, timer_int_o, 1'b0))
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

    Arbiter2_wb #(
        .DATA_WIDTH             (32),
        .ADDR_WIDTH             (32),
        .ARB_TYPE_ROUND_ROBIN   (0),
        .ARB_LSB_HIGH_PRIORITY  (0)
    ) arbiter (
        .clk        (wb_clk_i),
        .rst        (wb_rst_i),

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
    // wire [32*`SOC_XBAR_SLAVE_COUNT-1:0]  xbar_wb_adr_o;
    // wire [32*`SOC_XBAR_SLAVE_COUNT-1:0]  xbar_wb_dat_i;
    // wire [32*`SOC_XBAR_SLAVE_COUNT-1:0]  xbar_wb_dat_o;
    // wire [`SOC_XBAR_SLAVE_COUNT-1:0]     xbar_wb_we_o;
    // wire [4*`SOC_XBAR_SLAVE_COUNT-1:0]   xbar_wb_sel_o;
    // wire [`SOC_XBAR_SLAVE_COUNT-1:0]     xbar_wb_cyc_o;
    // wire [`SOC_XBAR_SLAVE_COUNT-1:0]     xbar_wb_stb_o;
    // wire [`SOC_XBAR_SLAVE_COUNT-1:0]     xbar_wb_ack_i;
    // wire [`SOC_XBAR_SLAVE_COUNT-1:0]     xbar_wb_err_i;

    `define listappend(macro, x) \
            `ifdef macro \
            , x \
            `endif

    Crossbar_wb #(
        .NSLAVES        (`SOC_XBAR_SLAVE_COUNT),
        .DATA_WIDTH     (32),
        .ADDR_WIDTH     (32),
        .DEVICE_ADDR    ({
            `SOC_BOOTROM_ADDR,
            `SOC_RAM_ADDR
            `listappend(SOC_EN_UART, `SOC_UART_ADDR)
            `listappend(SOC_EN_GPIO, `SOC_GPIO_ADDR)
            `listappend(SOC_EN_SPI, `SOC_SPI_ADDR)
            `listappend(SOC_EN_TIMER, `SOC_TIMER_ADDR)
        }),
        .DEVICE_MASK    ({
            `size_to_mask32(`SOC_BOOTROM_SIZE),
            `size_to_mask32(`SOC_RAM_SIZE)
            `listappend(SOC_EN_UART, `size_to_mask32(`SOC_UART_SIZE))
            `listappend(SOC_EN_GPIO, `size_to_mask32(`SOC_GPIO_SIZE))
            `listappend(SOC_EN_SPI, `size_to_mask32(`SOC_SPI_SIZE))
            `listappend(SOC_EN_TIMER, `size_to_mask32(`SOC_TIMER_SIZE))
        })
    ) xbar (
        .wb_clk_i       (wb_clk_i),

        .wbm_adr_i      (arb_wb_adr_o),
        .wbm_dat_i      (arb_wb_dat_o),
        .wbm_dat_o      (arb_wb_dat_i),
        .wbm_we_i       (arb_wb_we_o),
        .wbm_sel_i      (arb_wb_sel_o),
        .wbm_stb_i      (arb_wb_stb_o),
        .wbm_cyc_i      (arb_wb_cyc_o),
        .wbm_ack_o      (arb_wb_ack_i),
        .wbm_err_o      (arb_wb_err_i),

        .wbs_adr_o      ({
            bootrom_wb_adr_i, 
            ram_wb_adr_i
            `listappend(SOC_EN_UART, uart_wb_adr_i)
            `listappend(SOC_EN_GPIO, gpio_wb_adr_i)
            `listappend(SOC_EN_SPI, spi_wb_adr_i)
            `listappend(SOC_EN_TIMER, timer_wb_adr_i)
        }),
        .wbs_dat_i      ({
            bootrom_wb_dat_o,
            ram_wb_dat_o
            `listappend(SOC_EN_UART, uart_wb_dat_o)
            `listappend(SOC_EN_GPIO, gpio_wb_dat_o)
            `listappend(SOC_EN_SPI, spi_wb_dat_o)
            `listappend(SOC_EN_TIMER, timer_wb_dat_o)
        }),
        .wbs_dat_o      ({
            bootrom_wb_dat_i,
            ram_wb_dat_i
            `listappend(SOC_EN_UART, uart_wb_dat_i)
            `listappend(SOC_EN_GPIO, gpio_wb_dat_i)
            `listappend(SOC_EN_SPI, spi_wb_dat_i)
            `listappend(SOC_EN_TIMER, timer_wb_dat_i)
        }),
        .wbs_we_o       ({
            bootrom_wb_we_i,
            ram_wb_we_i
            `listappend(SOC_EN_UART, uart_wb_we_i)
            `listappend(SOC_EN_GPIO, gpio_wb_we_i)
            `listappend(SOC_EN_SPI, spi_wb_we_i)
            `listappend(SOC_EN_TIMER, timer_wb_we_i)
        }),
        .wbs_sel_o      ({
            bootrom_wb_sel_i,
            ram_wb_sel_i
            `listappend(SOC_EN_UART, uart_wb_sel_i)
            `listappend(SOC_EN_GPIO, gpio_wb_sel_i)
            `listappend(SOC_EN_SPI, spi_wb_sel_i)
            `listappend(SOC_EN_TIMER, timer_wb_sel_i)
        }),
        .wbs_cyc_o      ({
            bootrom_wb_cyc_i,
            ram_wb_cyc_i
            `listappend(SOC_EN_UART, uart_wb_cyc_i)
            `listappend(SOC_EN_GPIO, gpio_wb_cyc_i)
            `listappend(SOC_EN_SPI, spi_wb_cyc_i)
            `listappend(SOC_EN_TIMER, timer_wb_cyc_i)
        }),
        .wbs_stb_o      ({
            bootrom_wb_stb_i,
            ram_wb_stb_i
            `listappend(SOC_EN_UART, uart_wb_stb_i)
            `listappend(SOC_EN_GPIO, gpio_wb_stb_i)
            `listappend(SOC_EN_SPI, spi_wb_stb_i)
            `listappend(SOC_EN_TIMER, timer_wb_stb_i)
        }),
        .wbs_ack_i      ({
            bootrom_wb_ack_o,
            ram_wb_ack_o
            `listappend(SOC_EN_UART, uart_wb_ack_o)
            `listappend(SOC_EN_GPIO, gpio_wb_ack_o)
            `listappend(SOC_EN_SPI, spi_wb_ack_o)
            `listappend(SOC_EN_TIMER, timer_wb_ack_o)
        }),
        .wbs_err_i      ({
            bootrom_wb_err_o,
            ram_wb_err_o
            `listappend(SOC_EN_UART, uart_wb_err_o)
            `listappend(SOC_EN_GPIO, gpio_wb_err_o)
            `listappend(SOC_EN_SPI, spi_wb_err_o)
            `listappend(SOC_EN_TIMER, timer_wb_err_o)
        })
    );


    // ******************** BOOTROM ********************
    parameter BOOTROM_ADR_SIZE = $clog2(`SOC_BOOTROM_SIZE);

    wire  [31:0]    bootrom_wb_adr_i;
    `UNUSED_VAR(bootrom_wb_adr_i)
    wire  [31:0]    bootrom_wb_dat_o;
    wire  [31:0]    bootrom_wb_dat_i;
    `UNUSED_VAR(bootrom_wb_dat_i)
    wire 		    bootrom_wb_we_i;
    `UNUSED_VAR(bootrom_wb_we_i)
    wire  [3:0]     bootrom_wb_sel_i;
    `UNUSED_VAR(bootrom_wb_sel_i)
    wire            bootrom_wb_cyc_i;
    wire            bootrom_wb_stb_i;
    wire 		    bootrom_wb_ack_o;
    wire            bootrom_wb_err_o = 0;
    
    SinglePortROM_wb #(
        .ADDR_WIDTH (BOOTROM_ADR_SIZE),
        .MEM_FILE   (`SOC_BOOTROM_INIT_FILE)
    ) bootrom (
        .wb_clk_i   (wb_clk_i),
        .wb_rst_i   (wb_rst_i),
        .wb_adr_i   (bootrom_wb_adr_i[BOOTROM_ADR_SIZE-1:2]),
        .wb_dat_o   (bootrom_wb_dat_o),
        .wb_stb_i   (bootrom_wb_cyc_i & bootrom_wb_stb_i),
        .wb_ack_o   (bootrom_wb_ack_o)
    );


    // ******************** RAM ********************
    parameter RAM_ADR_SIZE = $clog2(`SOC_RAM_SIZE);

    wire  [31:0]    ram_wb_adr_i;
    `UNUSED_VAR(ram_wb_adr_i)
    wire  [31:0]    ram_wb_dat_o;
    wire  [31:0]    ram_wb_dat_i;
    wire 		    ram_wb_we_i;
    wire  [3:0]     ram_wb_sel_i;
    wire            ram_wb_cyc_i;
    wire            ram_wb_stb_i;
    wire 		    ram_wb_ack_o;
    wire            ram_wb_err_o = 0;

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
    `ifdef SOC_EN_UART
    wire  [31:0]    uart_wb_adr_i;
    `UNUSED_VAR(uart_wb_adr_i)
    wire  [31:0]    uart_wb_dat_o;
    wire  [31:0]    uart_wb_dat_i;
    wire 		    uart_wb_we_i;
    wire  [3:0]     uart_wb_sel_i;
    wire            uart_wb_cyc_i;
    wire            uart_wb_stb_i;
    wire 		    uart_wb_ack_o;
    wire            uart_wb_err_o = 0;
    
    UART #(
        .DEFAULT_DIV(1),
        .FIFO_EN    (`SOC_UART_FIFO_EN),
        .FIFO_DEPTH (`SOC_UART_FIFO_DEPTH)
    ) uart (
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
    `endif

    // ******************** GPIO ********************
    `ifdef SOC_EN_GPIO
    wire  [31:0]    gpio_wb_adr_i;
    `UNUSED_VAR(gpio_wb_adr_i)
    wire  [31:0]    gpio_wb_dat_o;
    wire  [31:0]    gpio_wb_dat_i;
    wire 		    gpio_wb_we_i;
    wire  [3:0]     gpio_wb_sel_i;
    wire            gpio_wb_cyc_i;
    wire            gpio_wb_stb_i;
    wire 		    gpio_wb_ack_o;
    wire            gpio_wb_err_o = 0;
    
    GPIO #(
        .N(`SOC_GPIO_NUM_PINS)
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
    `endif

    // ******************** SPI ********************
    `ifdef SOC_EN_SPI
    wire  [31:0]    spi_wb_adr_i;
    `UNUSED_VAR(spi_wb_adr_i)
    wire  [31:0]    spi_wb_dat_o;
    wire  [31:0]    spi_wb_dat_i;
    wire 		    spi_wb_we_i;
    wire  [3:0]     spi_wb_sel_i;
    wire            spi_wb_cyc_i;
    wire            spi_wb_stb_i;
    wire 		    spi_wb_ack_o;
    wire            spi_wb_err_o = 0;

    SPI_wb #(
        .NCS(`SOC_SPI_NUM_CS)
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
    `endif

    // ******************* TIMER *******************
    `ifdef SOC_EN_TIMER
    wire  [31:0]    timer_wb_adr_i;
    `UNUSED_VAR(timer_wb_adr_i)
    wire  [31:0]    timer_wb_dat_o;
    wire  [31:0]    timer_wb_dat_i;
    wire 		    timer_wb_we_i;
    wire  [3:0]     timer_wb_sel_i;
    wire            timer_wb_cyc_i;
    wire            timer_wb_stb_i;
    wire 		    timer_wb_ack_o;
    wire            timer_int_o;
    wire            timer_wb_err_o = 0;

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

    `ifndef EN_EXCEPT
    `UNUSED_VAR(timer_int_o)
    `endif // EN_EXCEPT
    
    `endif

endmodule
