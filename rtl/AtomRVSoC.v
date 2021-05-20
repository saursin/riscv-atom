////////////////////////////////////////////////////////////////////   
//  RISC-V Atom 
//
//  File        : AtomRVSoC.v
//  Author      : Saurabh Singh (saurabh.s99100@gmail.com)
//  Desciption  : System on Chip based on RISCV atom processor
////////////////////////////////////////////////////////////////////
`default_nettype none

`include "core/AtomRV.v"

module AtomRVSoC
(
    input        clk_i,    // external clock
    input        rst_i,    // external reset
    
    output [31:0] imem_addr_o,
    input [31:0] imem_data_i

    //input        uart_rxd,  // UART receive line
    //output       uart_txd,  // UART transmit line

    //output [3:0] leds,      // on-board leds
    //output [3:0] debug      // osciloscope
);
    //////////////////////////////////////////////////////
    // IMEM
    // wire [31:0] imem_addr;
    // wire [31:0] imem_data;

    //////////////////////////////////////////////////////
    // DMEM
    wire [31:0] d_addr_o /*verilator public*/;
    wire [31:0] d_data_i = 32'd0;
    wire [31:0] d_data_o;
    wire d_we_o;
    
    //////////////////////////////////////////////////////
    // Atom RISCV CORE
    wire hlt = 0;
    
    AtomRV atom
    (
        .clk_i(clk_i),    // clock
        .rst_i(rst_i),    // reset
        .hlt_i(hlt),    // hault cpu

        .i_data_i(imem_data_i),   // IMEM data
        .i_addr_o(imem_addr_o),   // IMEM Address
        .d_addr_o(d_addr_o),   // DMEM address
        .d_data_i(d_data_i),   // DMEM data in
        .d_data_o(d_data_o),   // DMEM data out
        .d_we_o(d_we_o)      // DMEM WriteEnable
    );


    //////////////////////////////////////////////////////
    // Tick Counter
    //reg [32:0]  TickCounter;

    //always @(posedge xclk_i) begin
      //  if(xrst_i) TickCounter = 32'd0;
        //else TickCounter = TickCounter + 1;
    //end
    //////////////////////////////////////////////////////
endmodule