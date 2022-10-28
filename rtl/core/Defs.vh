//////////////////////////////////////////////////////////////////
//  File        : Defs.v
//  Author      : Saurabh Singh (saurabh.s99100@gmail.com)
//  Description : Common Definitions for Atom core
//////////////////////////////////////////////////////////////////

`ifndef __DEFS_VH__
`define __DEFS_VH__


// Reset Address
`ifdef SOC_RESET_ADDRESS
    `define RESET_PC_ADDRESS    `SOC_RESET_ADDRESS
`else
    // Default
    `define RESET_PC_ADDRESS    32'h0000000
`endif


// NOP instruction (addi x0, x0, 0)
`define RV_INSTR_NOP        32'h00000013


// RISCV Immediate types
`define RV_IMM_TYPE_I   3'd0
`define RV_IMM_TYPE_S   3'd1
`define RV_IMM_TYPE_B   3'd2
`define RV_IMM_TYPE_U   3'd3
`define RV_IMM_TYPE_J   3'd4


// ALU
`define ALU_FUNC_ADD    3'd0
`define ALU_FUNC_SUB    3'd1
`define ALU_FUNC_XOR    3'd2
`define ALU_FUNC_OR     3'd3
`define ALU_FUNC_AND    3'd4
`define ALU_FUNC_SLL    3'd5
`define ALU_FUNC_SRL    3'd6
`define ALU_FUNC_SRA    3'd7


// Comparator Unit
`define CMP_FUNC_UN     3'd0
`define CMP_FUNC_EQ     3'd1
`define CMP_FUNC_NQ     3'd2
`define CMP_FUNC_LT     3'd3
`define CMP_FUNC_GE     3'd4
`define CMP_FUNC_LTU    3'd5
`define CMP_FUNC_GEU    3'd6


// ALU
`define ALU_FUNC_ADD 3'd0
`define ALU_FUNC_SUB 3'd1
`define ALU_FUNC_XOR 3'd2
`define ALU_FUNC_OR  3'd3
`define ALU_FUNC_AND 3'd4
`define ALU_FUNC_SLL 3'd5
`define ALU_FUNC_SRL 3'd6
`define ALU_FUNC_SRA 3'd7


`endif // __DEFS_VH__
