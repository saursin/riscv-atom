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
`include "Defs.vh"
`default_nettype none

module Alu
(
    input   wire    [31:0]  a_i,
    input   wire    [31:0]  b_i,
    input   wire    [3:0]   sel_i,

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
    
    
    /////// m extension
    wire sel_mul    =   (sel_i == `ALU_FUNC_MUL);
    wire sel_mulh   =   (sel_i == `ALU_FUNC_MULH);
    wire sel_mulhsu =   (sel_i == `ALU_FUNC_MULHSU);
    wire sel_mulhu  =   (sel_i == `ALU_FUNC_MULHU);
    wire sel_div    =   (sel_i == `ALU_FUNC_DIV);
    wire sel_divu   =   (sel_i == `ALU_FUNC_DIVU);
    wire sel_rem    =   (sel_i == `ALU_FUNC_REM);
    wire sel_remu   =   (sel_i == `ALU_FUNC_REMU);

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
     //mul,mulh,mulhsu,mulhu
    // M-Extension mux
    wire [63:0] result_mul;
    /* verilator lint_off UNUSED */
    wire [63:0] result_mulsu;
    /* verilator lint_off UNUSED */
    wire [63:0] result_mulu;
    /* verilator lint_off UNUSED */
    wire [31:0] result_div;
    wire [31:0] result_divu;
    wire [31:0] result_rem;
    wire [31:0] result_remu;
    
    assign result_mul[63:0]     =   $signed  ({{32{a_i[31]}}, a_i[31: 0]}) *
                                    $signed  ({{32{b_i[31]}}, b_i[31: 0]});

    assign result_mulu[63:0]    =   $unsigned  ({{32{1'b0}}, a_i[31: 0]}) *
                                    $unsigned  ({{32{1'b0}}, b_i[31: 0]});
    
    assign result_mulsu[63:0]   =   $signed  ({{32{a_i[31]}}, a_i[31: 0]}) *
                                    $unsigned  ({{32{1'b0}}, b_i[31: 0]});

    assign result_div[31:0]     =   (b_i == 32'h00000000) ? 32'hffffffff :
                                    ((a_i == 32'h80000000) && (b_i == 32'hffffffff)) ? 32'h80000000 :
                                    $signed  ($signed  (a_i) / $signed  (b_i));

    assign result_divu[31:0]    =   (b_i == 32'h00000000) ? 32'hffffffff :
                                    $unsigned($unsigned(a_i) / $unsigned(b_i));

    assign result_rem[31:0]     =   (b_i == 32'h00000000) ? a_i :
                                    ((a_i == 32'h80000000) && (b_i == 32'hffffffff)) ? 32'h00000000 :
                                    $signed  ($signed  (a_i) % $signed  (b_i));

    assign result_remu[31: 0]   =   (b_i == 32'h00000000) ? a_i :
                                    $unsigned($unsigned(a_i) % $unsigned(b_i));  


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
        else if(sel_mul)                             // M start
            result_o = result_mul[31:0];
        else if(sel_mulh)
            result_o = result_mul[63:32];
        else if(sel_mulhsu)
            result_o = result_mulsu[63:32];
        else if(sel_mulhu)
            result_o = result_mulu[63:32];
        else if(sel_div)
            result_o = result_div[31:0];
        else if(sel_divu)
            result_o = result_divu[31:0];
        else if(sel_rem)
            result_o = result_rem[31:0];
        else if(sel_remu)
            result_o = result_remu[31:0]; 
        else
            result_o = arith_result;
    end        

endmodule
