///////////////////////////////////////////////////////////////////
//  File        : AtomBones.v                                                        
//  Author      : Saurabh Singh (saurabh.s99100@gmail.com)
//  Description : AtomBones is a wrapper for AtomRV_wb, it is a 
//      stub SoC target for AtomSim, which uses software simulated 
//      memories and communication modules.
///////////////////////////////////////////////////////////////////
`default_nettype none
`include "AtomBones_Config.vh"

module AtomBones
(
    input   wire    clk_i,
    input   wire    rst_i,

    output  wire    [31:0]  iport_addr_o,    // IMEM Address
    input   wire    [31:0]  iport_data_i,    // IMEM data

    output  wire            iport_valid_o,   // IMEM Valid signal
    input   wire            iport_ack_i,     // IMEM Ack signal

    output  wire    [31:0]  dport_addr_o,    // DMEM address
    input   wire    [31:0]  dport_data_i,    // DMEM data in
    output  wire    [31:0]  dport_data_o,    // DMEM data out
    output  wire    [3:0]   dport_sel_o,     // DMEM Select
    output  wire            dport_we_o,      // DMEM Strobe

    output  wire            dport_valid_o,   // DMEM Valid signal
    input   wire            dport_ack_i      // DMEM Ack signal
);

    reg [31:0] reset_vector = `SOC_RESET_ADDRESS;

    // Core
    AtomRV atom_core
    (
        .clk_i          (clk_i),   
        .rst_i          (rst_i),

        .reset_vector_i (reset_vector),
        
        .iport_addr_o    (iport_addr_o),   
        .iport_data_i    (iport_data_i),   
        .iport_valid_o   (iport_valid_o),   
        .iport_ack_i     (iport_ack_i),   
        
        .dport_addr_o    (dport_addr_o),   
        .dport_data_i    (dport_data_i),   
        .dport_data_o    (dport_data_o),   
        .dport_sel_o     (dport_sel_o),   
        .dport_we_o      (dport_we_o),   
        .dport_valid_o   (dport_valid_o),   
        .dport_ack_i     (dport_ack_i)

        `ifdef EN_EXCEPT
        ,
        .irq_i          (1'b0),
        .timer_int_i    (1'b0)
        `endif // EN_EXCEPT
    );

endmodule
