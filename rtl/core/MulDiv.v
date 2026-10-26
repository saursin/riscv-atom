////////////////////////////////////////////////////////////////////
//  File        : MulDiv.v
//  Description : M-extension execution unit for Atom core.
//      MUL/MULH/MULHSU/MULHU : single cycle (33x33 signed multiply)
//      DIV/DIVU/REM/REMU     : iterative divider (rtl/lib/Divider.v);
//                              stall_o stays high until the result is ready
//
//      kill_i abandons an in-flight division (its instruction was
//      flushed by a trap); the late result is ignored.
////////////////////////////////////////////////////////////////////
`include "Defs.vh"
`default_nettype none

module MulDiv
(
    input   wire            clk_i,
    input   wire            rst_i,

    input   wire            valid_i,    // an M instruction is in stage2
    input   wire    [3:0]   op_i,       // ALU_FUNC_MUL .. ALU_FUNC_REMU
    input   wire            kill_i,     // instruction in stage2 abandoned

    input   wire    [31:0]  a_i,
    input   wire    [31:0]  b_i,

    output  wire    [31:0]  result_o,
    output  wire            stall_o     // result not ready yet
);
    wire sel_mul    = (op_i == `ALU_FUNC_MUL);
    wire sel_mulh   = (op_i == `ALU_FUNC_MULH);
    wire sel_mulhsu = (op_i == `ALU_FUNC_MULHSU);
    wire sel_mulhu  = (op_i == `ALU_FUNC_MULHU);
    wire sel_div    = (op_i == `ALU_FUNC_DIV);
    wire sel_divu   = (op_i == `ALU_FUNC_DIVU);
    wire sel_rem    = (op_i == `ALU_FUNC_REM);
    wire sel_remu   = (op_i == `ALU_FUNC_REMU);

    ////////////////////////////////////////////////////////////////
    // Multiply
    wire signed_a  = sel_mul | sel_mulh | sel_mulhsu;
    wire signed_b  = sel_mul | sel_mulh;    // Only mulhu is unsigned*unsigned

    wire use_high_result = sel_mulh | sel_mulhsu | sel_mulhu;

    // 33x33 signed multiply covers all four variants (unsigned operands are
    // zero-extended); its low 64 bits are the full 32x32 product.
    wire signed [32:0] mul_a_ext = {signed_a & a_i[31], a_i};
    wire signed [32:0] mul_b_ext = {signed_b & b_i[31], b_i};
    /* verilator lint_off UNUSED */
    wire signed [65:0] mul_full = mul_a_ext * mul_b_ext;
    /* verilator lint_on UNUSED */
    wire [31:0] mul_result = use_high_result ? mul_full[63:32] : mul_full[31:0];

    ////////////////////////////////////////////////////////////////
    // Divide
    wire is_div_op = sel_div | sel_divu | sel_rem | sel_remu;
    wire is_div    = valid_i & is_div_op;

    wire            div_busy;
    wire            div_done;
    wire    [31:0]  div_quotient;
    wire    [31:0]  div_remainder;
    reg             div_pending;    // a division was issued for the current instruction

    wire div_start = is_div && !div_busy && !div_pending && !kill_i;
    wire div_valid = div_done && div_pending;

    always @(posedge clk_i) begin
        if (rst_i)
            div_pending <= 1'b0;
        else if (kill_i)
            div_pending <= 1'b0;
        else if (div_start)
            div_pending <= 1'b1;
        else if (div_done)
            div_pending <= 1'b0;
    end

    Divider #(
        .WIDTH  (32)
    ) divider (
        .clk_i          (clk_i),
        .rst_i          (rst_i),
        .start_i        (div_start),
        .signed_i       (sel_div | sel_rem),
        .dividend_i     (a_i),
        .divisor_i      (b_i),
        .busy_o         (div_busy),
        .done_o         (div_done),
        .quotient_o     (div_quotient),
        .remainder_o    (div_remainder)
    );

    wire [31:0] div_result = (sel_rem | sel_remu) ? div_remainder : div_quotient;

    ////////////////////////////////////////////////////////////////
    // Outputs
    assign result_o = is_div_op ? div_result : mul_result;
    assign stall_o  = is_div && !div_valid;

endmodule
