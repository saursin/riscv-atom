////////////////////////////////////////////////////////////////////
//  File        : Divider.v
//  Description : Iterative (radix-2 restoring) integer divider with
//      early completion paths. Quotient and remainder are computed
//      together; operands are signed or unsigned per operation.
//
//      Latency (cycles from the edge sampling start_i to done_o):
//        divisor == 0 : 1   (quotient = all ones, remainder = dividend;
//                            matches RISC-V DIV/DIVU/REM/REMU)
//        otherwise    : 2 + WIDTH - SKIP_W*k, where k is the number of
//                       leading all-zero SKIP_W-bit chunks of |dividend|
//                       (dividend == 0 => 2)
//
//      Signed overflow (MIN / -1) yields quotient = MIN, remainder = 0.
//
//      Handshake: pulse start_i while busy_o is low. done_o pulses for
//      one cycle when quotient_o/remainder_o are valid; they hold until
//      the next start.
////////////////////////////////////////////////////////////////////
`default_nettype none

module Divider #(
    parameter WIDTH  = 32,
    parameter SKIP_W = 8        // leading-zero skip granularity; must divide WIDTH
)(
    input   wire                clk_i,
    input   wire                rst_i,

    input   wire                start_i,
    input   wire                signed_i,
    input   wire [WIDTH-1:0]    dividend_i,
    input   wire [WIDTH-1:0]    divisor_i,

    output  wire                busy_o,
    output  reg                 done_o,
    output  reg  [WIDTH-1:0]    quotient_o,
    output  reg  [WIDTH-1:0]    remainder_o
);
    localparam NCHUNK = WIDTH / SKIP_W;
    localparam SKIPW  = $clog2(NCHUNK+1);
    localparam CNTW   = $clog2(WIDTH+1);

    localparam S_IDLE   = 2'd0;
    localparam S_RUN    = 2'd1;
    localparam S_FINISH = 2'd2;

    reg [1:0] state;
    assign busy_o = (state != S_IDLE);

    ////////////////////////////////////////////////////////////////
    // Operand preparation (start cycle)
    wire dividend_neg = signed_i & dividend_i[WIDTH-1];
    wire divisor_neg  = signed_i & divisor_i[WIDTH-1];
    wire [WIDTH-1:0] dividend_abs = dividend_neg ? -dividend_i : dividend_i;
    wire [WIDTH-1:0] divisor_abs  = divisor_neg  ? -divisor_i  : divisor_i;

    // Leading all-zero chunks of |dividend| (NCHUNK when dividend == 0).
    // Their quotient bits are zero and leave the remainder at zero, so
    // they are skipped by pre-shifting the dividend.
    reg [SKIPW-1:0] skip;
    reg             found;
    integer c;
    always @(*) begin
        skip  = {SKIPW{1'b0}};
        found = 1'b0;
        for (c = NCHUNK-1; c >= 0; c = c-1) begin
            if (!found && (dividend_abs[c*SKIP_W +: SKIP_W] == {SKIP_W{1'b0}}))
                skip = skip + 1'b1;
            else
                found = 1'b1;
        end
    end

    /* verilator lint_off UNUSED */
    wire [31:0]     iters_full = WIDTH - skip * SKIP_W;
    /* verilator lint_on UNUSED */
    wire [CNTW-1:0] iters = iters_full[CNTW-1:0];

    ////////////////////////////////////////////////////////////////
    // Iteration datapath
    reg [WIDTH-1:0] rem;    // partial remainder
    reg [WIDTH-1:0] quo;    // dividend bits shift out on top, quotient bits shift in below
    reg [WIDTH-1:0] dvs;    // |divisor|
    reg [CNTW-1:0]  cnt;    // iterations left
    reg             neg_q;
    reg             neg_r;

    wire [WIDTH:0]  rem_sh = {rem, quo[WIDTH-1]};
    wire [WIDTH:0]  diff   = rem_sh - {1'b0, dvs};
    wire            fits   = ~diff[WIDTH];

    always @(posedge clk_i) begin
        if (rst_i) begin
            state       <= S_IDLE;
            done_o      <= 1'b0;
            quotient_o  <= {WIDTH{1'b0}};
            remainder_o <= {WIDTH{1'b0}};
        end
        else begin
            done_o <= 1'b0;
            case (state)
                S_IDLE: begin
                    if (start_i) begin
                        if (divisor_i == {WIDTH{1'b0}}) begin
                            quotient_o  <= {WIDTH{1'b1}};
                            remainder_o <= dividend_i;
                            done_o      <= 1'b1;
                        end
                        else begin
                            rem   <= {WIDTH{1'b0}};
                            quo   <= dividend_abs << (skip * SKIP_W);
                            dvs   <= divisor_abs;
                            cnt   <= iters;
                            neg_q <= dividend_neg ^ divisor_neg;
                            neg_r <= dividend_neg;
                            state <= (iters == {CNTW{1'b0}}) ? S_FINISH : S_RUN;
                        end
                    end
                end

                S_RUN: begin
                    rem <= fits ? diff[WIDTH-1:0] : rem_sh[WIDTH-1:0];
                    quo <= {quo[WIDTH-2:0], fits};
                    cnt <= cnt - 1'b1;
                    if (cnt == {{(CNTW-1){1'b0}}, 1'b1})
                        state <= S_FINISH;
                end

                S_FINISH: begin
                    quotient_o  <= neg_q ? -quo : quo;
                    remainder_o <= neg_r ? -rem : rem;
                    done_o      <= 1'b1;
                    state       <= S_IDLE;
                end

                default:
                    state <= S_IDLE;
            endcase
        end
    end

endmodule
