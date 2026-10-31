////////////////////////////////////////////////////////////////////
//  File        : HydrogenSoC_sim.v
//  Description : atomsim's top for HydrogenSoC: the SoC unchanged, plus the parts of a
//                board that atomsim models in RTL rather than in C++. So far that is the
//                DDR3 PHY and chip (rtl/uncore/ddr/ddr3_phy_model.sv) when the config
//                enables peripherals.ddr. Otherwise the same ports as HydrogenSoC.
////////////////////////////////////////////////////////////////////
`default_nettype none
`include "atom_config.vh"
`ifdef SOC_EN_DDR
`include "ddr3_phy_model.sv"
`endif

module HydrogenSoC_sim(
    input   wire    clk_i,
    input   wire    rst_i

`ifdef SOC_EN_UART
    ,
    input   wire    uart_rx_i,
    output  wire    uart_tx_o
`endif

`ifdef SOC_EN_GPIO
    ,
    input   wire [`SOC_GPIO_NUM_PINS-1:0]   gpio_inp_i,
    output  wire [`SOC_GPIO_NUM_PINS-1:0]   gpio_out_o,
    output  wire [`SOC_GPIO_NUM_PINS-1:0]   gpio_dir_o
`endif

`ifdef SOC_EN_SPI
    ,
    input   wire                            spi_miso_i,
    output  wire                            spi_mosi_o,
    output  wire                            spi_sck_o,
    output  wire [`SOC_SPI_NUM_CS-1:0]      spi_cs_o
`endif
);

`ifdef SOC_EN_DDR
    localparam DDR_DQ_W = `SOC_DDR_DQ_BITS;
    localparam DDR_REFI = `SOC_DDR_CLK_FREQ / 1000 * 78 / 10000 / 2;     // tREFI, controller cycles

    wire                        ddr_ce, ddr_reset_n, ddr_cke, ddr_odt, ddr_wr, ddr_rd, ddr_rvalid, ddr_ready;
    wire [2:0]                  ddr_cmd;
    wire [`SOC_DDR_ROW_BITS-1:0] ddr_addr;
    wire [`SOC_DDR_BANK_BITS-1:0] ddr_ba;
    wire [1:0]                  ddr_wr_ph, ddr_rd_ph, ddr_wr_shift;
    wire [8*DDR_DQ_W-1:0]       ddr_wdata, ddr_rdata;
    wire [DDR_DQ_W-1:0]         ddr_wmask;
    wire [DDR_DQ_W/8*3-1:0]     ddr_rd_delay;
`endif

    HydrogenSoC soc (
        .clk_i      (clk_i),
        .rst_i      (rst_i)
`ifdef SOC_EN_UART
        ,
        .uart_rx_i  (uart_rx_i),
        .uart_tx_o  (uart_tx_o)
`endif
`ifdef SOC_EN_GPIO
        ,
        .gpio_inp_i (gpio_inp_i),
        .gpio_out_o (gpio_out_o),
        .gpio_dir_o (gpio_dir_o)
`endif
`ifdef SOC_EN_SPI
        ,
        .spi_miso_i (spi_miso_i),
        .spi_mosi_o (spi_mosi_o),
        .spi_sck_o  (spi_sck_o),
        .spi_cs_o   (spi_cs_o)
`endif
`ifdef SOC_EN_DDR
        ,
        .ddr_phy_ce_i       (ddr_ce),
        .ddr_phy_reset_n_o  (ddr_reset_n),
        .ddr_phy_cke_o      (ddr_cke),
        .ddr_phy_odt_o      (ddr_odt),
        .ddr_phy_cmd_o      (ddr_cmd),
        .ddr_phy_addr_o     (ddr_addr),
        .ddr_phy_ba_o       (ddr_ba),
        .ddr_phy_wr_o       (ddr_wr),
        .ddr_phy_wr_ph_o    (ddr_wr_ph),
        .ddr_phy_wdata_o    (ddr_wdata),
        .ddr_phy_wmask_o    (ddr_wmask),
        .ddr_phy_rd_o       (ddr_rd),
        .ddr_phy_rd_ph_o    (ddr_rd_ph),
        .ddr_phy_rd_delay_o (ddr_rd_delay),
        .ddr_phy_wr_shift_o (ddr_wr_shift),
        .ddr_phy_rvalid_i   (ddr_rvalid),
        .ddr_phy_rdata_i    (ddr_rdata),
        .ddr_phy_ready_i    (ddr_ready)
`endif
    );

`ifdef SOC_EN_DDR
    // DDR3 PHY and chip; fails the simulation if refreshes are more than 2 x tREFI apart
    ddr3_phy_model #(
        .DQ_W(DDR_DQ_W), .ROW_W(`SOC_DDR_ROW_BITS), .COL_W(`SOC_DDR_COL_BITS),
        .BA_W(`SOC_DDR_BANK_BITS), .CL(6), .DW(3), .REFI_MAX(2 * DDR_REFI)
    ) ddr_phy (
        .clk            (clk_i),
        .ce             (ddr_ce),
        .phy_reset_n    (ddr_reset_n),
        .phy_cke        (ddr_cke),
        .phy_cmd        (ddr_cmd),
        .phy_addr       (ddr_addr),
        .phy_ba         (ddr_ba),
        .phy_wr         (ddr_wr),
        .phy_wdata      (ddr_wdata),
        .phy_wmask      (ddr_wmask),
        .phy_rd         (ddr_rd),
        .phy_rd_delay   (ddr_rd_delay),
        .phy_rvalid     (ddr_rvalid),
        .phy_rdata      (ddr_rdata),
        .phy_ready      (ddr_ready),
        .phy_odt        (ddr_odt),
        .phy_wr_ph      (ddr_wr_ph),
        .phy_rd_ph      (ddr_rd_ph),
        .phy_wr_shift   (ddr_wr_shift)
    );
`endif

endmodule
