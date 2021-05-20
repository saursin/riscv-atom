///////////////////////////////////////////////////////////////////
//      ____  _________ _______    __         __                  
//     / __ \/  _/ ___// ____/ |  / /  ____ _/ /_____  ____ ___   
//    / /_/ // / \__ \/ /    | | / /  / __ `/ __/ __ \/ __ `__ \  
//   / _, _// / ___/ / /___  | |/ /  / /_/ / /_/ /_/ / / / / / /  
//  /_/ |_/___//____/\____/  |___/   \__,_/\__/\____/_/ /_/ /_/   
//                                                                
//  File       : AtomRV.v                                                        
//  Author     : Saurabh Singh (saurabh.s99100@gmail.com)
//  Description: Nano-RISCV core is a non pipelined embedded class 
//               32 bit RISCV core. It is based on RV32I ISA
//  
///////////////////////////////////////////////////////////////////
`default_nettype none

`include "../Timescale.vh"
`include "../Config.vh"

`include "Decode.v"
`include "mux32x4.v"
`include "RegisterFile.v"
`include "Alu.v"

module AtomRV
(
    input clk_i,    // clock
    input rst_i,    // reset
    input hlt_i,    // hault cpu

    input   [31:0]  i_data_i,   // IMEM data
    output  [31:0]  i_addr_o,   // IMEM Address

    output  [31:0]  d_addr_o,   // DMEM address
    input   [31:0]  d_data_i,   // DMEM data in
    output  [31:0]  d_data_o,   // DMEM data out
    output          d_we_o      // DMEM WriteEnable
);

////////////////////////////////////////////////////////////////////
localparam FETCH    = 1'b0;
localparam EXECUTE  = 1'b1;

/*
    This register is used to store current state of processor
    Possible states : FETCH, EXECUTE
*/
reg ProcessorState  /*verilator public*/;
always @(posedge clk_i) begin 
    if(rst_i)
        ProcessorState <= EXECUTE;
    else if(!hlt_i)
        ProcessorState <= ! ProcessorState;
end


/*
    Program Counter
*/
reg [31:0] ProgramCounter   /*verilator public*/;
wire [31:0] pc_plus_four = ProgramCounter + 4;
always @(posedge clk_i) begin 
    if(rst_i)
        ProgramCounter <= `RESET_PC_ADDRESS;
    else if(d_pc_we)
        ProgramCounter <= alu_out;
    else if(ProcessorState == EXECUTE)
        ProgramCounter <= pc_plus_four;
end
assign i_addr_o = ProgramCounter;

/*
    This register is used to store current instruction that is being 
    executed
*/
reg [31:0] InstructionRegister  /*verilator public*/;
always @(posedge clk_i) begin
    if(rst_i)
        InstructionRegister <= `__NOP_INSTRUCTION__;
    else if(ProcessorState == FETCH)
        InstructionRegister <= i_data_i;
end


/*
    Instruction Decode
*/
wire    [4:0]   d_rd_sel;
wire    [4:0]   d_rs1_sel;
wire    [4:0]   d_rs2_sel;
wire    [31:0]  d_imm;

wire            d_pc_we;
wire            d_rf_we;
wire            d_a_op_sel;
wire            d_b_op_sel;
wire    [3:0]   d_alu_op_sel;
wire    [1:0]   d_rf_din_sel;

wire branch_taken;

Decode decode
(
    .instr_i         (InstructionRegister),
    .branch_taken_i  (branch_taken),
    
    .rd_sel_o        (d_rd_sel),
    .rs1_sel_o       (d_rs1_sel),
    .rs2_sel_o       (d_rs2_sel),
    
    .imm_o           (d_imm),

    .pc_we_o         (d_pc_we),
    .rf_we_o         (d_rf_we),
    .rf_din_sel_o    (d_rf_din_sel),
    .a_op_sel_o      (d_a_op_sel),
    .b_op_sel_o      (d_b_op_sel),
    .alu_op_sel_o    (d_alu_op_sel)
);


/*
    Regster File
*/
wire    [31:0]  rf_rd_data;
mux32x4 mux(d_imm, pc_plus_four, alu_out, 32'd0, d_rf_din_sel, rf_rd_data); // MUX;

wire    [31:0]  rf_rs1;
wire    [31:0]  rf_rs2;

RegisterFile  #(.REG_WIDTH(32), .REG_ADDR_WIDTH(5)) rf
(
    .Ra_Sel_i   (d_rs1_sel),
    .Ra_o       (rf_rs1),

    .Rb_Sel_i   (d_rs2_sel),
    .Rb_o       (rf_rs2),

    .Data_We_i  (d_rf_we),
    .Rd_Sel_i   (d_rd_sel),
    .Data_i     (rf_rd_data),

    .Clk_i      (clk_i),
    .Rst_i      (rst_i)
);


/*
    Alu
*/
wire    [31:0]  alu_a_in = (d_a_op_sel) ? ProgramCounter : rf_rs1;
wire    [31:0]  alu_b_in = (d_b_op_sel) ? d_imm : rf_rs2;
wire    [31:0]  alu_out;

Alu alu
(
    .A      (alu_a_in),
    .B      (alu_b_in),
    .Sel    (d_alu_op_sel),
    .Out    (alu_out)
);

assign branch_taken = alu_out[0];


assign d_addr_o = d_data_i;   // DMEM address

assign d_data_o = 32'd0;   // DMEM data out
assign d_we_o   =  1'b0;   // DMEM WriteEnable

endmodule