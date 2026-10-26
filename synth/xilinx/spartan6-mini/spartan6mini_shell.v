////////////////////////////////////////////////////////////////////
//  File        : spartan6mini_shell.v
//  Description : Spartan6-mini board shell for HydrogenSoC:
//      reset polarity and synchronization, GPIO pads. Port names match
//      the pin constraints (HydrogenSoC.ucf).
////////////////////////////////////////////////////////////////////
`default_nettype none

module spartan6mini_shell (
    input   wire                            clk_i,
    input   wire                            rst_i,          // active-high push button

    input   wire                            uart_rx_i,
    output  wire                            uart_tx_o,

    inout   wire [`SOC_GPIO_NUM_PINS-1:0]   gpio_io,

    input   wire                            spi_miso_i,
    output  wire                            spi_mosi_o,
    output  wire                            spi_sck_o,
    output  wire [`SOC_SPI_NUM_CS-1:0]      spi_cs_o
);
    // Reset: active-high button -> synchronized active-high reset
    wire soc_rst;
    reset_sync #(
        .NUM_STAGES (2)
    ) rst_sync (
        .clk_i  (clk_i),
        .rst_i  (rst_i),
        .rst_o  (soc_rst)
    );

    // GPIO pads
    wire [`SOC_GPIO_NUM_PINS-1:0] gpio_inp;
    wire [`SOC_GPIO_NUM_PINS-1:0] gpio_out;
    wire [`SOC_GPIO_NUM_PINS-1:0] gpio_dir;

    IOBank #(
        .NUM_PINS   (`SOC_GPIO_NUM_PINS)
    ) iob (
        .dat_i      (gpio_out),
        .dir_i      (gpio_dir),
        .out_o      (gpio_inp),
        .pins_io    (gpio_io)
    );

    HydrogenSoC soc (
        .clk_i      (clk_i),
        .rst_i      (soc_rst),
        .uart_rx_i  (uart_rx_i),
        .uart_tx_o  (uart_tx_o),
        .gpio_inp_i (gpio_inp),
        .gpio_out_o (gpio_out),
        .gpio_dir_o (gpio_dir),
        .spi_miso_i (spi_miso_i),
        .spi_mosi_o (spi_mosi_o),
        .spi_sck_o  (spi_sck_o),
        .spi_cs_o   (spi_cs_o)
    );

endmodule
