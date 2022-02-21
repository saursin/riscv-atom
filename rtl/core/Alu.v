////////////////////////////////////////////////////////////////////   
//  File        : Alu.v
//  Author      : Saurabh Singh (saurabh.s99100@gmail.com)
//  Description : Arithmetic and logic unit for Atom core, following 
//      Operations are built into the alu:
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

    output  reg     [31:0]  result_o
);

    wire sel_add = (sel_i == `ALU_FUNC_ADD);
    wire sel_sub = (sel_i == `ALU_FUNC_SUB);
    wire sel_xor = (sel_i == `ALU_FUNC_XOR);
    wire sel_or  = (sel_i == `ALU_FUNC_OR);
    wire sel_and = (sel_i == `ALU_FUNC_AND);
    wire sel_sll = (sel_i == `ALU_FUNC_SLL);
    wire sel_srl = (sel_i == `ALU_FUNC_SRL);
    wire sel_sra = (sel_i == `ALU_FUNC_SRA);

    // Result of arithmetic calculations (ADD/SUB)
    wire [31:0] arith_result = a_i + (sel_sub ? ((~b_i)+1) : b_i);

    // Bitreverse
    function [31:0] reverse;
        input [31:0]    ain;
        integer i;
        begin
            for(i=0; i<32; i=i+1) begin
                reverse[32-1-i] = ain[i];
            end
        end
    endfunction
    
    // Input to the universal shifter
    reg signed [32:0] shift_input;
    always @(*) begin
        if (sel_srl)
            shift_input = {1'b0, a_i};
        else if (sel_sra)
            shift_input = {a_i[31], a_i};
        else // if (sel_sll) // this case includes "sel_sll"
            shift_input = {1'b0, reverse(a_i)};
    end

    /* verilator lint_off UNUSED */
    wire [32:0] shift_output = shift_input >>> b_i[4:0];    // Universal shifter
    /* verilator lint_on UNUSED */


    // output of universal shifter
    reg [31:0] final_shift_output;
    always @(*) begin
        if (sel_sll)
            final_shift_output = reverse(shift_output[31:0]);
        else
            final_shift_output = shift_output[31:0];
    end
    
    // Final output mux
    always @(*) begin
        if (sel_add | sel_sub)
            result_o = arith_result;
        else if (sel_sll | sel_srl | sel_sra)
            result_o = final_shift_output;
        else if (sel_xor)
            result_o = a_i ^ b_i;
        else if (sel_or)
            result_o = a_i | b_i;
        else if (sel_and)
            result_o = a_i & b_i;
        else
            result_o = arith_result;
    end        

endmodule