////////////////////////////////////////////////////////////////////   
//  RISC-V Atom 
//
//  File        : Decode.v
//  Author      : Saurabh Singh (saurabh.s99100@gmail.com)
//  Desciption  : Instruction Decoder for RISCV atom core
////////////////////////////////////////////////////////////////////
`default_nettype none

`include "../Timescale.vh"

module Decode
(
    input   [31:0]  instr_i,
    input           branch_taken_i,

    output  [4:0]   rd_sel_o,
    output  [4:0]   rs1_sel_o,
    output  [4:0]   rs2_sel_o,

    output  [31:0]  imm_o,

    output  reg         pc_we_o,
    output  reg         rf_we_o,
    output  reg [1:0]   rf_din_sel_o,
    output  reg         a_op_sel_o,
    output  reg         b_op_sel_o,
    output  reg [3:0]   alu_op_sel_o
);

// Decode fields
wire    [6:0]   opcode  = instr_i[6:0];
wire    [2:0]   func3   = instr_i[14:12];
//wire    [6:0]   func7   = instr_i[31:25];

assign  rd_sel_o    = instr_i[11:7];
assign  rs1_sel_o   = instr_i[19:15];
assign  rs2_sel_o   = instr_i[24:20];

reg    [2:0] instr_format;

/*
    Decode Immidiate
*/
function [31:0] getExtImm;
    input   [31:0]  instr /* verilator lint_off UNUSED */;
    input   [2:0]   fmt;

    begin
        case(fmt)
            `__I_IMMIDIATE__: getExtImm = {{21{instr[31]}}, instr[30:25], instr[24:21], instr[20]};
            `__S_IMMIDIATE__: getExtImm = {{21{instr[31]}}, instr[30:25], instr[11:8], instr[7]};
            `__B_IMMIDIATE__: getExtImm = {{20{instr[31]}}, instr[7], instr[30:25], instr[11:8], 1'b0};
            `__U_IMMIDIATE__: getExtImm = {instr[31], instr[30:20], instr[19:12], {12{1'b0}}};
            `__J_IMMIDIATE__: getExtImm = {{12{instr[31]}}, instr[19:12], instr[20], instr[30:25], instr[24:21], 1'b0};

            default:
                getExtImm = {{25{instr[6]}}, instr[6:0]};
        endcase
    end
endfunction

assign imm_o = getExtImm(instr_i, instr_format);




always @(*) begin
    casez({func3, opcode})
        /* LUI   */ 10'b???_0110111: begin
            rf_din_sel_o = 2'd0;
            rf_we_o = 1'b1;
            instr_format = `__U_IMMIDIATE__;
        end
        /* AUIPC */ 10'b???_0010111: begin
            rf_din_sel_o = 2'd2;
            a_op_sel_o = 1'b1;
            b_op_sel_o = 1'b1;
            rf_we_o = 1'b1;
            instr_format = `__U_IMMIDIATE__;
            alu_op_sel_o = `__ALU_ADD__;
        end
        // /* JAL   */ 10'b???_1101111: begin
        //     pc_we_o = 1'b1;
        //     rf_we_o = 1'b1;
        //     rf_din_sel_o = 2'b1;
        //     instr_format = `__J_IMMIDIATE__;
        // end
        // /* JALR  */ 10'b000_1100111: begin
        //     pc_we_o = 1'b1;
        //     rf_we_o = 1'b1;
        //     rf_din_sel_o = 2'b1;
        //     instr_format = `__I_IMMIDIATE__;
        // end
         /* BEQ   */ 10'b000_1100011: begin         // NOT VERIFIED YET
             pc_we_o = branch_taken_i;
             rf_we_o = branch_taken_i;
             rf_din_sel_o = 2'b1;
             instr_format = `__I_IMMIDIATE__;
        end
    //    /* BNE   */ 10'b001_1100011: 
    //    /* BLT   */ 10'b100_1100011: 
    //    /* BGE   */ 10'b101_1100011: 
    //    /* BLTU  */ 10'b110_1100011: 
    //    /* BGEU  */ 10'b111_1100011: 
    //    /* LB    */ 10'b000_0000011: 
    //    /* LH    */ 10'b001_0000011: 
    //    /* LW    */ 10'b010_0000011: 
    //    /* LBU   */ 10'b100_0000011: 
    //    /* LHU   */ 10'b101_0000011: 
    //    /* SB    */ 10'b000_0100011: 
    //    /* SH    */ 10'b001_0100011: 
    //    /* SW    */ 10'b010_0100011: 
        /* ADDI  */ 10'b000_0010011: begin
            rf_din_sel_o = 2'd2;
            rf_we_o = 1'b1;
            a_op_sel_o = 1'b0;
            b_op_sel_o = 1'b1;
            alu_op_sel_o = `__ALU_ADD__;
            instr_format = `__I_IMMIDIATE__;
        end
    //    /* SLTI  */ 10'b010_0010011: 
    //    /* SLTIU */ 10'b011_0010011: 
    //    /* XORI  */ 10'b100_0010011: 
    //    /* ORI   */ 10'b110_0010011: 
    //    /* ANDI  */ 10'b111_0010011: 
    //    /* SLLI  */ 10'b001_0010011: 
    //    /* SRLI  */ 10'b101_0010011: 
    //    /* SRAI  */ 10'b101_0010011: 
        /* ADD   */ 10'b000_0110011: begin
            rf_din_sel_o = 2'd2;
            rf_we_o = 1'b1;
            a_op_sel_o = 1'b0;
            b_op_sel_o = 1'b0;
            alu_op_sel_o = `__ALU_ADD__;
        end
        /* SUB   */ 10'b000_0110011: begin
            rf_din_sel_o = 2'd2;
            rf_we_o = 1'b1;
            a_op_sel_o = 1'b0;
            b_op_sel_o = 1'b0;
            alu_op_sel_o = `__ALU_SUB__;
        end
    //    /* SLL   */ 10'b001_0110011: 
    //    /* SLT   */ 10'b010_0110011: 
    //    /* SLTU  */ 10'b011_0110011: 
    //    /* XOR   */ 10'b100_0110011: 
    //    /* SRL   */ 10'b101_0110011: 
    //    /* SRA   */ 10'b101_0110011: 
    //    /* OR    */ 10'b110_0110011: 
    //    /* AND   */ 10'b111_0110011: 

        default: begin
            pc_we_o = 0;
            rf_we_o = 0;
            rf_din_sel_o = 0;
            a_op_sel_o = 0;
            b_op_sel_o = 0;
            alu_op_sel_o = 0;
        end            

    endcase
end








































/*
0000 pred succ 00000 000 00000 0001111 FENCE
0000 0000 0000 00000 001 00000 0001111 FENCE.I
000000000000 00000 000 00000 1110011 ECALL
000000000001 00000 000 00000 1110011 EBREAK
csr rs1 001 rd 1110011 CSRRW
csr rs1 010 rd 1110011 CSRRS
csr rs1 011 rd 1110011 CSRRC
csr zimm 101 rd 1110011 CSRRWI
csr zimm 110 rd 1110011 CSRRSI
csr zimm 111 rd 1110011 CSRRCI
*/

endmodule