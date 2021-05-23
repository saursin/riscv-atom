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

`define __R0_IS_ZERO__

`include "../Timescale.vh"
`include "../Config.vh"

`include "Decode.v"
`include "RegisterFile.v"
`include "Alu.v"

module AtomRV
(
    input clk_i,    // clock
    input rst_i,    // reset
    input hlt_i,    // hault cpu

    input   [31:0]  imem_data_i,   // IMEM data
    output  [31:0]  imem_addr_o,   // IMEM Address

    output  [31:0]  dmem_addr_o,            // DMEM address
    input   [31:0]  dmem_data_i,            // DMEM data in
    output  [31:0]  dmem_data_o,            // DMEM data out
    output  [2:0]   dmem_access_width_o,    // DMEM Access width
    output          dmem_we_o               // DMEM WriteEnable
);

wire jump_decision = d_jump_en & comparison_result; // final jump decision signal

////////////////////////////////////////////////////////////////////
//  STAGE 1 - FETCH
////////////////////////////////////////////////////////////////////
/*
    Program Counter
*/
reg [31:0] ProgramCounter   /*verilator public*/;
wire [31:0] pc_plus_four = ProgramCounter + 32'd4;

always @(posedge clk_i) begin 
    if(rst_i)
        ProgramCounter <= `RESET_PC_ADDRESS;

    else if(jump_decision)
        ProgramCounter <= {alu_out[31:1], 1'b0};    // Only jump to 16 bit aligned addrressesm, also JALR enforces this

    else
        ProgramCounter <= pc_plus_four;
end

// Connect pc to imem address input
assign imem_addr_o = ProgramCounter;


//-------------------------------
// PIPELINE REGISTERS
//-------------------------------
/*
    This register is used to store old value of program counter
*/
reg [31:0]  ProgramCounter_Old /* verilator public */;
always @(posedge clk_i) begin 
    if(rst_i)
        ProgramCounter_Old <= 32'd0;
    else
        ProgramCounter_Old <= ProgramCounter;
end

/*
    This register is used to store old value of program counter + 4
*/
reg [31:0]  link_address;
always @(posedge clk_i) begin 
    if(rst_i)
        link_address <= 32'd0;
    else
        link_address <= pc_plus_four;
end

/*
    This register is used to store current instruction that is being 
    executed
*/
reg [31:0] InstructionRegister  /*verilator public*/;
always @(posedge clk_i) begin
    if(rst_i)
        InstructionRegister <= `__NOP_INSTRUCTION__;
    else
        InstructionRegister <= (jump_decision) ? `__NOP_INSTRUCTION__ : imem_data_i;   // Stall
end



////////////////////////////////////////////////////////////////////
//  STAGE 2 - DECODE & EXECUTE
////////////////////////////////////////////////////////////////////
/*
    Instruction Decode
*/
wire    [4:0]   d_rd_sel;
wire    [4:0]   d_rs1_sel;
wire    [4:0]   d_rs2_sel;
wire    [31:0]  d_imm;

wire            d_jump_en;
wire    [2:0]   d_comparison_type;
wire            d_rf_we;
wire    [2:0]   d_rf_din_sel;
wire            d_a_op_sel;
wire            d_b_op_sel;
wire            d_cmp_b_op_sel;
wire    [2:0]   d_alu_op_sel;
wire    [2:0]   d_mem_access_width;
wire            d_mem_we;


Decode decode
(
    .instr_i            (InstructionRegister),
    
    .rd_sel_o           (d_rd_sel),
    .rs1_sel_o          (d_rs1_sel),
    .rs2_sel_o          (d_rs2_sel),
    
    .imm_o              (d_imm),

    .jump_en_o          (d_jump_en),
    .comparison_type_o  (d_comparison_type),
    .rf_we_o            (d_rf_we),
    .rf_din_sel_o       (d_rf_din_sel),
    .a_op_sel_o         (d_a_op_sel),
    .b_op_sel_o         (d_b_op_sel),
    .cmp_b_op_sel_o     (d_cmp_b_op_sel),
    .alu_op_sel_o       (d_alu_op_sel),
    .mem_access_width_o (d_mem_access_width),
    .mem_we_o           (d_mem_we)
);


/*
    MEM_LOAD
*/
function [31:0] memload;
    input   [31:0]  value;
    input   [2:0]   load_type;

    begin
        case(load_type)
            3'b000:   memload = {{24{value[7]}}, value[7:0]};     // LB
            3'b001:   memload = {{16{value[15]}}, value[15:0]};   // LH
            3'b010:   memload = value;                            // LW
            3'b100:   memload = {{24{1'b0}}, value[7:0]};         // LBU
            3'b101:   memload = {{16{1'b0}}, value[15:0]};        // LHU

            default: memload = 32'd0;
        endcase 
    end
endfunction

/*
    Regster File
*/

// RF_Din Multiplexer
reg    [31:0]  rf_rd_data;
always @(*) begin
    case(d_rf_din_sel)
        3'd0:   rf_rd_data = d_imm;
        3'd1:   rf_rd_data = link_address;
        3'd2:   rf_rd_data = alu_out;
        3'd3:   rf_rd_data = {31'd0, comparison_result};
        3'd4:   rf_rd_data = memload(dmem_data_i, d_mem_access_width);

        default: rf_rd_data = 32'd0;
    endcase
end

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
wire    [31:0]  alu_a_in = (d_a_op_sel) ? ProgramCounter_Old : rf_rs1;
wire    [31:0]  alu_b_in = (d_b_op_sel) ? d_imm : rf_rs2;
wire    [31:0]  alu_out;

Alu alu
(
    .A      (alu_a_in),
    .B      (alu_b_in),
    .Sel    (d_alu_op_sel),
    .Out    (alu_out)
);



function compare;
    input   [31:0]  A;
    input   [31:0]  B;
    input   [2:0]   comparison_type;

    reg signed  [31:0]  A_signed = A;
    reg signed  [31:0]  B_signed = B;
    begin
        case(comparison_type)
            `__CMP_UN__   :   compare = 1'b1;
            `__CMP_EQ__   :   compare = (A == B);
            `__CMP_NQ__   :   compare = (A != B);
            `__CMP_LT__   :   compare = (A_signed < B_signed);
            `__CMP_GE__   :   compare = (A_signed >= B_signed);
            `__CMP_LTU__  :   compare = (A < B);
            `__CMP_GEU__  :   compare = (A >= B);

            default:    compare = 1'b0;
        endcase
    end
endfunction

wire comparison_result = compare
                        (
                            rf_rs1, 
                            ((d_cmp_b_op_sel) ? d_imm : rf_rs2), 
                            d_comparison_type
                        );


/*
    DATA MEMORY ACCESS
*/
assign dmem_addr_o = alu_out;
assign dmem_data_o = rf_rs1;
assign dmem_access_width_o = d_mem_access_width;
assign dmem_we_o = d_mem_we;

endmodule