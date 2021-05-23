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
    input [2:0]     Sel,

    output reg [31:0] Out
);

always @(*) begin
    case(Sel)

    `__ALU_ADD__:   Out = A + B;
    `__ALU_SUB__:   Out = A - B; 
    `__ALU_XOR__:   Out = A ^ B;
    `__ALU_OR__ :   Out = A | B;
    `__ALU_AND__:   Out = A & B;
    `__ALU_SLL__:   Out = A << B;
    `__ALU_SRL__:   Out = A >> B;
    `__ALU_SRA__:   Out = A >>> B;

    default:    Out = 32'd0;
    endcase
end
endmodule