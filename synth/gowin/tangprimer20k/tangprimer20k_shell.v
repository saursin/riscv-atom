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
    );

endmodule

`default_nettype wire
