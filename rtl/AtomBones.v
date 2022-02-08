///////////////////////////////////////////////////////////////////
//  File        : AtomBones.v                                                        
//  Author      : Saurabh Singh (saurabh.s99100@gmail.com)
//  Description : AtomBones is a wrapper for AtomRV_wb, it is a 
//      stub SoC target for AtomSim, which uses software simulated 
//      memories and communication modules.
///////////////////////////////////////////////////////////////////

`include "Timescale.vh"
`include "Config.vh"

`include "core/AtomRV.v"

`default_nettype none

 module AtomBones
(
    input   wire    clk_i,
    input   wire    rst_i,

    output  wire    [31:0]  imem_addr_o,    // IMEM Address
    input   wire    [31:0]  imem_data_i,    // IMEM data

    output  wire            imem_valid_o,   // IMEM Valid signal
    input   wire            imem_ack_i,     // IMEM Ack signal

    output  wire    [31:0]  dmem_addr_o,    // DMEM address
    input   wire    [31:0]  dmem_data_i,    // DMEM data in
    output  wire    [31:0]  dmem_data_o,    // DMEM data out
    output  wire    [3:0]   dmem_sel_o,     // DMEM Select
    output  wire            dmem_we_o,      // DMEM Strobe

    output  wire            dmem_valid_o,   // DMEM Valid signal
    input   wire            dmem_ack_i      // DMEM Ack signal
);

    // Core
    AtomRV atom_core
    (
        .clk_i          (clk_i),   
        .rst_i          (rst_i),   
        .imem_addr_o    (imem_addr_o),   
        .imem_data_i    (imem_data_i),   
        .imem_valid_o   (imem_valid_o),   
        .imem_ack_i     (imem_ack_i),   
        .dmem_addr_o    (dmem_addr_o),   
        .dmem_data_i    (dmem_data_i),   
        .dmem_data_o    (dmem_data_o),   
        .dmem_sel_o     (dmem_sel_o),   
        .dmem_we_o      (dmem_we_o),   
        .dmem_valid_o   (dmem_valid_o),   
        .dmem_ack_i     (dmem_ack_i)
    );

endmodule