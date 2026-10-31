////////////////////////////////////////////////////////////////////
//  File        : tangprimer20k_shell.v
//  Description : Sipeed Tang Primer 20K board shell for HydrogenSoC.
//      Ports are board features (tangprimer20k_shell.cst); the shell maps
//      the SoC's clock, reset and GPIO onto them:
//          clock: 27 MHz oscillator -> PLL -> 45 MHz (27 * 5 / 3)
//          GPIO[5:0]   -> LEDs (active low)
//          GPIO[7:6]  <-> PMOD0[1:0] (bidirectional)
//          GPIO[11:8] <-  DIP switches 1-4 (GPIO[9:8] = bootmode)
//          GPIO[15:12]<-  buttons S1-S4 (active low)
//      Button S0 is the reset.
////////////////////////////////////////////////////////////////////
`default_nettype none
`include "atom_config.vh"

module tangprimer20k_shell (
    input   wire            clk_27mhz_i,

    input   wire            uart_rx_i,
    output  wire            uart_tx_o,

    output  wire [5:0]      led_n_o,
    input   wire [4:0]      btn_n_i,
    input   wire [3:0]      sw_i,

    output  wire            flash_spi_sck_o,
    output  wire            flash_spi_cs_o,
    output  wire            flash_spi_mosi_o,
    input   wire            flash_spi_miso_i,

    inout   wire [1:0]      pmod0_io

`ifdef SOC_EN_DDR
    ,
    // DDR3 chip (16-bit, 1Gb): pins and IO standards in tangprimer20k_shell.cst
    output  wire            ddr_ck,
    output  wire            ddr_ck_n,
    output  wire            ddr_reset_n,
    output  wire            ddr_cke,
    output  wire            ddr_cs_n,
    output  wire            ddr_ras_n,
    output  wire            ddr_cas_n,
    output  wire            ddr_we_n,
    output  wire            ddr_odt,
    output  wire [2:0]      ddr_ba,
    output  wire [13:0]     ddr_addr,
    output  wire [1:0]      ddr_dm,
    inout   wire [1:0]      ddr_dqs,
    inout   wire [1:0]      ddr_dqs_n,
    inout   wire [15:0]     ddr_dq
`endif
);
    // Clock: 27 MHz oscillator -> PLL -> 45 MHz (tangprimer20k_pll.v, generated)
    wire clk;
    wire pll_lock;
    tangprimer20k_pll pll (
        .clock_in   (clk_27mhz_i),
        .clock_out  (clk),
        .locked     (pll_lock)
    );

    // Reset: button S0 (active low) or PLL not locked -> synchronized active-high reset
    wire rst;
    reset_sync #(
        .NUM_STAGES (2)
    ) rst_sync (
        .clk_i  (clk),
        .rst_i  (~btn_n_i[0] | ~pll_lock),
        .rst_o  (rst)
    );

    // GPIO mapping
    wire [`SOC_GPIO_NUM_PINS-1:0] gpio_inp;
    wire [`SOC_GPIO_NUM_PINS-1:0] gpio_out;
    wire [`SOC_GPIO_NUM_PINS-1:0] gpio_dir;

    assign led_n_o          = ~gpio_out[5:0];
    assign gpio_inp[5:0]    = gpio_out[5:0];      // LEDs read back their own value

    IOBank #(
        .NUM_PINS   (2)
    ) pmod0_iob (
        .dat_i      (gpio_out[7:6]),
        .dir_i      (gpio_dir[7:6]),
        .out_o      (gpio_inp[7:6]),
        .pins_io    (pmod0_io)
    );

    assign gpio_inp[11:8]   = sw_i;
    assign gpio_inp[15:12]  = ~btn_n_i[4:1];

`ifdef SOC_EN_DDR
    // DDR3 PHY (gowin-ddr3 ddr3_phy_slow): every pin from a register on the SoC clock,
    // CK = clk / 2, DLL-off DRAM; CK and DQS through ELVDS buffers (LVCMOS15D pins)
    localparam DDR_DQ_W = `SOC_DDR_DQ_BITS;

    wire                            ddr_ce, ddr_phy_reset_n, ddr_phy_cke, ddr_phy_odt;
    wire                            ddr_phy_wr, ddr_phy_rd, ddr_phy_rvalid, ddr_phy_ready;
    wire [2:0]                      ddr_phy_cmd;
    wire [`SOC_DDR_ROW_BITS-1:0]    ddr_phy_addr;
    wire [`SOC_DDR_BANK_BITS-1:0]   ddr_phy_ba;
    wire [1:0]                      ddr_phy_wr_ph, ddr_phy_rd_ph, ddr_phy_wr_shift;
    wire [8*DDR_DQ_W-1:0]           ddr_phy_wdata, ddr_phy_rdata;
    wire [DDR_DQ_W-1:0]             ddr_phy_wmask;
    wire [DDR_DQ_W/8*3-1:0]         ddr_phy_rd_delay;

    // the 1Gb chip has 13 row bits; the unused address pins stay low
    wire [`SOC_DDR_ROW_BITS-1:0]    ddr_row_addr;
    assign ddr_addr = {{(14-`SOC_DDR_ROW_BITS){1'b0}}, ddr_row_addr};

    ddr3_phy_slow #(
        .DQ_W   (DDR_DQ_W),
        .ROW_W  (`SOC_DDR_ROW_BITS),
        .BA_W   (`SOC_DDR_BANK_BITS),
        .CL     (6),
        .CWL    (6),
        .DW     (3),
        .ELVDS  (1)
    ) ddr_phy (
        .clk            (clk),
        .ce             (ddr_ce),
        .phy_reset_n    (ddr_phy_reset_n),
        .phy_cke        (ddr_phy_cke),
        .phy_odt        (ddr_phy_odt),
        .phy_cmd        (ddr_phy_cmd),
        .phy_addr       (ddr_phy_addr),
        .phy_ba         (ddr_phy_ba),
        .phy_wr         (ddr_phy_wr),
        .phy_wr_ph      (ddr_phy_wr_ph),
        .phy_wdata      (ddr_phy_wdata),
        .phy_wmask      (ddr_phy_wmask),
        .phy_rd         (ddr_phy_rd),
        .phy_rd_ph      (ddr_phy_rd_ph),
        .phy_rd_delay   (ddr_phy_rd_delay),
        .phy_wr_shift   (ddr_phy_wr_shift),
        .phy_rvalid     (ddr_phy_rvalid),
        .phy_rdata      (ddr_phy_rdata),
        .phy_ready      (ddr_phy_ready),
        .dqs_in         (),
        .ddr_ck         (ddr_ck),
        .ddr_ck_n       (ddr_ck_n),
        .ddr_reset_n    (ddr_reset_n),
        .ddr_cke        (ddr_cke),
        .ddr_cs_n       (ddr_cs_n),
        .ddr_ras_n      (ddr_ras_n),
        .ddr_cas_n      (ddr_cas_n),
        .ddr_we_n       (ddr_we_n),
        .ddr_odt        (ddr_odt),
        .ddr_ba         (ddr_ba),
        .ddr_addr       (ddr_row_addr),
        .ddr_dm         (ddr_dm),
        .ddr_dqs        (ddr_dqs),
        .ddr_dqs_n      (ddr_dqs_n),
        .ddr_dq         (ddr_dq)
    );
`endif

    HydrogenSoC soc (
        .clk_i      (clk),
        .rst_i      (rst),
        .uart_rx_i  (uart_rx_i),
        .uart_tx_o  (uart_tx_o),
        .gpio_inp_i (gpio_inp),
        .gpio_out_o (gpio_out),
        .gpio_dir_o (gpio_dir),
        .spi_miso_i (flash_spi_miso_i),
        .spi_mosi_o (flash_spi_mosi_o),
        .spi_sck_o  (flash_spi_sck_o),
        .spi_cs_o   (flash_spi_cs_o)
`ifdef SOC_EN_DDR
        ,
        .ddr_phy_ce_i       (ddr_ce),
        .ddr_phy_reset_n_o  (ddr_phy_reset_n),
        .ddr_phy_cke_o      (ddr_phy_cke),
        .ddr_phy_odt_o      (ddr_phy_odt),
        .ddr_phy_cmd_o      (ddr_phy_cmd),
        .ddr_phy_addr_o     (ddr_phy_addr),
        .ddr_phy_ba_o       (ddr_phy_ba),
        .ddr_phy_wr_o       (ddr_phy_wr),
        .ddr_phy_wr_ph_o    (ddr_phy_wr_ph),
        .ddr_phy_wdata_o    (ddr_phy_wdata),
        .ddr_phy_wmask_o    (ddr_phy_wmask),
        .ddr_phy_rd_o       (ddr_phy_rd),
        .ddr_phy_rd_ph_o    (ddr_phy_rd_ph),
        .ddr_phy_rd_delay_o (ddr_phy_rd_delay),
        .ddr_phy_wr_shift_o (ddr_phy_wr_shift),
        .ddr_phy_rvalid_i   (ddr_phy_rvalid),
        .ddr_phy_rdata_i    (ddr_phy_rdata),
        .ddr_phy_ready_i    (ddr_phy_ready)
`endif
    );

endmodule

`default_nettype wire
