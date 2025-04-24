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

`ifdef EN_RVM
    wire sel_mul    = (sel_i == `ALU_FUNC_MUL);
    wire sel_mulh   = (sel_i == `ALU_FUNC_MULH);
    wire sel_mulhsu = (sel_i == `ALU_FUNC_MULHSU);
    wire sel_mulhu  = (sel_i == `ALU_FUNC_MULHU);
    wire sel_div    = (sel_i == `ALU_FUNC_DIV);
    wire sel_divu   = (sel_i == `ALU_FUNC_DIVU);
    wire sel_rem    = (sel_i == `ALU_FUNC_REM);
    wire sel_remu   = (sel_i == `ALU_FUNC_REMU);
`endif // EN_RVM

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

`ifdef EN_RVM
    // Control signals for multiply
    wire signed_a  = sel_mul  | sel_mulh | sel_mulhsu;
    wire signed_b  = sel_mul  | sel_mulh;  // Only mulhu is unsigned*unsigned

    wire use_high_result = sel_mulh | sel_mulhsu | sel_mulhu;

    wire [63:0] mul_a_ext = signed_a ? {{32{a_i[31]}}, a_i} : {32'b0, a_i};
    wire [63:0] mul_b_ext = signed_b ? {{32{b_i[31]}}, b_i} : {32'b0, b_i};
    wire [63:0] mul_result = mul_a_ext * mul_b_ext;
    wire [31:0] mul_final = use_high_result ? mul_result[63:32] : mul_result[31:0];

    wire signed [31:0] div_a_s = a_i;
    wire signed [31:0] div_b_s = b_i;
    wire        [31:0] div_a_u = a_i;
    wire        [31:0] div_b_u = b_i;

    reg signed [31:0] div_squotient, div_sremainder;
    always @(*) begin
    if (div_b_s == 0) begin
        div_squotient = -1;         // maps to 32'hFFFF_FFFF
        div_sremainder = div_a_s;
    end
    else if (div_a_s == -32'sd2147483648 && div_b_s == -1) begin
        // overflow corner: (–2³¹) ÷ (–1)
        div_squotient  = -32'sd2147483648;
        div_sremainder =  0;
    end
    else begin
        div_squotient  = div_a_s / div_b_s;
        div_sremainder = div_a_s % div_b_s;
    end
    end

    wire [31:0] div_uquotient  = (div_b_u == 0)
                                ? 32'hFFFF_FFFF     // DIVU /0 => all-ones
                                : div_a_u / div_b_u;

    wire [31:0] div_uremainder = (div_b_u == 0)
                                ? div_a_u           // REMU /0 => dividend
                                : div_a_u % div_b_u;

    wire        div_signed = sel_div | sel_rem;
    wire [31:0] div_quotient  = div_signed ? div_squotient  : div_uquotient;
    wire [31:0] div_remainder = div_signed ? div_sremainder : div_uremainder;
`endif // EN_RVM

    
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
    `ifdef EN_RVM
        else if (sel_mul | sel_mulh | sel_mulhsu | sel_mulhu)
            result_o = mul_final;
        else if (sel_div | sel_divu)
            result_o = div_quotient;
        else if (sel_rem | sel_remu)
            result_o = div_remainder;
    `endif // EN_RVM
        else
            result_o = arith_result;
    end

endmodule
