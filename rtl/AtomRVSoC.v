////////////////////////////////////////////////////////////////////   
//  RISC-V Atom 
//
//  File        : AtomRVSoC.v
//  Author      : Saurabh Singh (saurabh.s99100@gmail.com)
//  Desciption  : System on Chip based on RISCV atom processor
////////////////////////////////////////////////////////////////////
`default_nettype none
`include "Timescale.vh"
`include "core/AtomRV.v"
`include "uncore/SinglePortRAM.v"
`include "uncore/GPIO.v"

module AtomRVSoC
(
    input   wire            clk_i,    // external clock
    input   wire            rst_i,    // external reset
    


    //input        uart_rxd,  // UART receive line
    //output       uart_txd,  // UART transmit line

    //output [3:0] leds,      // on-board leds
    //output [3:0] debug      // osciloscope
);
    
    //////////////////////////////////////////////////////
    // IMEM (8Kb)
    wire    [31:0]      imem_wb_adr_i;
    wire    [31:0]      imem_wb_dat_i;
    wire    [3:0]       imem_wb_sel_i;
    wire                imem_wb_we_i;
    wire                imem_wb_cyc_i;
    wire    [31:0]      imem_wb_rdt_o;
    wire                imem_wb_ack_o;
    
    SinglePortRAM #(
        // Parameters
        .ADDR_WIDTH (16),
        .MEM_FILE   ("imem_init.hex")
    ) imem (
        // Wishbone Interface
        .wb_clk_i   (clk_i),
        .wb_rst_i   (rst_i),

        .wb_adr_i   (imem_wb_adr_i),
        .wb_dat_i   (imem_wb_dat_i),
        .wb_sel_i   (imem_wb_sel_i),
        .wb_we_i    (imem_wb_we_i), 
        .wb_cyc_i   (imem_wb_cyc_i),  
        .wb_rdt_o   (imem_wb_rdt_o),  
        .wb_ack_o   (imem_wb_ack_o)
    );


    //////////////////////////////////////////////////////
    // DMEM
    wire    [31:0]      dmem_wb_adr_i;
    wire    [31:0]      dmem_wb_dat_i;
    wire    [3:0]       dmem_wb_sel_i;
    wire                dmem_wb_we_i;
    wire                dmem_wb_cyc_i;
    wire    [31:0]      dmem_wb_rdt_o;
    wire                dmem_wb_ack_o;
    
    SinglePortRAM #(
        // Parameters
        .ADDR_WIDTH (16),
        .MEM_FILE   ("dmem_init.hex")
    ) dmem (
        // Wishbone Interface
        .wb_clk_i   (clk_i),
        .wb_rst_i   (rst_i),

        .wb_adr_i   (dmem_wb_adr_i),
        .wb_dat_i   (dmem_wb_dat_i),
        .wb_sel_i   (dmem_wb_sel_i),
        .wb_we_i    (dmem_wb_we_i), 
        .wb_cyc_i   (dmem_wb_cyc_i),  
        .wb_rdt_o   (dmem_wb_rdt_o),  
        .wb_ack_o   (dmem_wb_ack_o)
    );

    //////////////////////////////////////////////////////
    // RISC-V Atom Core (With Wishbone interface)
    
    AtomRV_wb atom
    (
        .clk_i                  (clk_i),    // clock
        .rst_i                  (rst_i),    // reset

        .imem_addr_o            (imem_addr_o),      // IMEM Address
        .imem_data_i            (imem_data_i),      // IMEM data

        .imem_valid_o           (imem_valid_o),
        .imem_ack_i             (imem_ack_i),

        .dmem_addr_o            (dmem_addr_o),      // DMEM address
        .dmem_data_i            (dmem_data_i),      // DMEM data in
        .dmem_data_o            (dmem_data_o),      // DMEM data out   
        .dmem_sel_o             (dmem_sel_o),       // DMEM Strobe
        .dmem_we_o              (dmem_we_o),        // DMEM WriteEnable

        .dmem_valid_o           (dmem_valid_o),     // DMEM valid
        .dmem_ack_i             (dmem_ack_i)        // DMEM ack
    );

endmodule