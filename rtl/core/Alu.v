////////////////////////////////////////////////////////////////////   
//  RISC-V Atom 
//
//  File : Alu.v
//
//  Desciption : Arithmetic and logic unit for Atom core
////////////////////////////////////////////////////////////////////
`default_nettype none

`include "../Timescale.vh"

module Alu
(
    input [31:0]    A,
    input [31:0]    B,
    input [3:0]     Sel,

    output reg [31:0] Out
);

wire signed [31:0] A_Signed = A;
wire signed [31:0] B_Signed = B;

always @(*) begin
    case(Sel)

    `__ALU_ADD__:   Out = A + B;
    `__ALU_SUB__:   Out = A - B; 
    `__ALU_XOR__:   Out = A ^ B;
    `__ALU_OR__ :   Out = A | B;
    `__ALU_AND__:   Out = A & B;
    `__ALU_EQ__ :   Out = A == B ? 32'd1 : 32'd0;
    `__ALU_NQ__ :   Out = A != B ? 32'd1 : 32'd0;
    `__ALU_LT__ :   Out = A_Signed < B_Signed ? 32'd1 : 32'd0;
    `__ALU_GE__ :   Out = A_Signed >= B_Signed ? 32'd1 : 32'd0;
    `__ALU_LTU__:   Out = A < B ? 32'd1 : 32'd0;
    `__ALU_GEU__:   Out = A >= B ? 32'd1 : 32'd0;

        default:    Out = 32'd0;
    endcase
end
endmodule