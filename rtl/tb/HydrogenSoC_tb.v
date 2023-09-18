`default_nettype none

`include "HydrogenSoC_Config.vh"

module isim_tb;
    reg clk_i = 0;
    reg rst_i = 1;

    // GPIO
    wire [`NGPIO-1:0] gpio_io;

    // UART
    reg     uart_mux_sel_i = 0;
    reg     uart_usb_rx_i = 0;
    wire    uart_usb_tx_o;
    reg     uart_io_rx_i = 0;
    wire    uart_io_tx_o;

    // SPI
    reg     spi_miso_i = 0;
    wire    spi_mosi_o;
    wire    spi_sck_o;
    wire    spi_cs_o;

    // DUT    
    HydrogenSoC hosc(
        clk_i,
        rst_i,
        gpio_io,
        uart_mux_sel_i,
        uart_usb_rx_i,
        uart_usb_tx_o,
        uart_io_rx_i,
        uart_io_tx_o,
        spi_miso_i,
        spi_mosi_o,
        spi_sck_o,
        spi_cs_o
    );

    initial begin
        $dumpfile("trace.vcd");
        $dumpvars(0, isim_tb);
        #10;
        rst_i = 0;
    end

    reg [63:0] clk_ctr = 0;
    always @(posedge clk_i) begin
        clk_ctr <= clk_ctr + 1;

        if(clk_ctr == 200000)
            $finish(0);
    end

    always begin
        #1; clk_i <= ~clk_i;
    end
endmodule