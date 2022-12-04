`default_nettype none
`timescale 1ns/1ps

`include "../HydrogenSoC_Config.vh"

module isim_tb;
reg clk_i = 0;
reg rst_i = 1;

wire [`NGPIO-1:0] gpio_io;

// UART
reg uart_usb_rx_i = 0;
wire uart_usb_tx_o;

reg  uart_io_rx_i = 0;
wire uart_io_tx_o;

// UART MUX
reg uart_mux_sel = 0;
    
// TEST POINTS
wire uart_rx_test_point_o;
wire uart_tx_test_point_o;

HydrogenSoC hsoc (
    clk_i,
    rst_i,
    gpio_io,
    uart_usb_rx_i,
    uart_usb_tx_o,
    uart_io_rx_i,
    uart_io_tx_o,
    uart_mux_sel,
    uart_rx_test_point_o,
    uart_tx_test_point_o
);


initial begin
    #10;
    rst_i = 0;
end

always begin
    #1;
    clk_i <= ~clk_i;
end
endmodule