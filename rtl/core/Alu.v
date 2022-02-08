////////////////////////////////////////////////////////////////////   
//  RISC-V Atom 
//
//  File : Alu.v
//
//  Description : Arithmetic and logic unit for Atom core
////////////////////////////////////////////////////////////////////
`include "../Timescale.vh"
`include "Defs.vh"

`default_nettype none

module Alu
(
    input   wire    [31:0]  A,
    input   wire    [31:0]  B,
    input   wire    [2:0]   Sel,

    output reg [31:0] Out
);

wire signed [31:0] A_s = A;

always @(*) begin
    case(Sel)

    `ALU_FUNC_ADD:   Out = A + B;
    `ALU_FUNC_SUB:   Out = A - B; 
    `ALU_FUNC_XOR:   Out = A ^ B;
    `ALU_FUNC_OR:    Out = A | B;
    `ALU_FUNC_AND:   Out = A & B;
    `ALU_FUNC_SLL:   Out = A << B[4:0];
    `ALU_FUNC_SRL:   Out = A >> B[4:0];
    `ALU_FUNC_SRA:   Out = A_s >>> B[4:0];

    default:    Out = 32'd0;
    endcase
end
endmodule