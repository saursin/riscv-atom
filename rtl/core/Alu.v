////////////////////////////////////////////////////////////////////   
//  File : Alu.v
//
//  Description : Arithmetic and logic unit for Atom core
//      Operations supported:
//          -   Addition
//          -   Subtraction
//          -   Bitwise XOR
//          -   Bitwise OR,
//          -   Bitwise AND,
//          -   Logical Shift Left (Single Cycle)
//          -   Logical Shift Right (Single Cycle)
//          -   Arthmetic Shift Right (Single Cycle)
////////////////////////////////////////////////////////////////////

`include "../Timescale.vh"
`include "Defs.vh"

`default_nettype none

module Alu
(
    input   wire    [31:0]  a_i,
    input   wire    [31:0]  b_i,
    input   wire    [2:0]   sel_i,

    output reg [31:0] result_o
);

    wire signed [31:0] A_s = a_i;

    always @(*) begin
        case(sel_i)

            `ALU_FUNC_ADD:  result_o = a_i + b_i;
            `ALU_FUNC_SUB:  result_o = a_i - b_i; 
            `ALU_FUNC_XOR:  result_o = a_i ^ b_i;
            `ALU_FUNC_OR:   result_o = a_i | b_i;
            `ALU_FUNC_AND:  result_o = a_i & b_i;
            `ALU_FUNC_SLL:  result_o = a_i << b_i[4:0];
            `ALU_FUNC_SRL:  result_o = a_i >> b_i[4:0];
            `ALU_FUNC_SRA:  result_o = A_s >>> b_i[4:0];

            default:    result_o = 32'd0;
        endcase
    end

endmodule