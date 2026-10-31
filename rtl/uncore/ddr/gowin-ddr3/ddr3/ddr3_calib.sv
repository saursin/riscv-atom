`ifndef module_ddr3_calib
`define module_ddr3_calib

// DDR3 read calibration.
//
// Writes a BL8 test pattern to address 0, then reads it back for every read
// delay step the PHY offers (0..STEPS-1, increasing steps sample later) and
// picks the middle of the longest window of steps that read the pattern
// correctly CAL_REPEAT times in a row. With PER_LANE the window is searched
// for each byte lane separately, otherwise all lanes get the common window.
//
// Neighbouring beats of the pattern are bitwise complements, so sampling half
// a beat off flips every bit, and all beats of a byte lane differ, so a whole
// beat off fails too and a dump of the read back data shows where each byte
// came from.
//
// A PHY whose write data can end up whole clocks off DQS (WR_STEPS > 1) gets
// wr_shift: when no read delay works, the pattern is written again with the
// next shift and the sweep repeats. A write that is off loses beats, so the
// read back can't pass by accident.
module ddr3_calib #(
    parameter DQ_W     = 32,
    parameter ROW_W    = 15,
    parameter COL_W    = 10,
    parameter BA_W     = 3,
    parameter DW       = 3,             // read delay bits per lane
    parameter STEPS    = 8,
    parameter PER_LANE = 1,
    parameter REPEAT   = 4,
    parameter WR_STEPS = 1
) (
    input                       clk,
    input                       ce,
    input                       rst,
    input                       start,

    output reg                  done,
    output reg                  fail,
    output reg [DQ_W/8*DW-1:0]  rd_delay,
    output reg            [1:0] wr_shift,
    input                       phy_ready,

    output reg                  op_valid,
    input                       op_ready,
    output reg            [2:0] op,
    output     [BA_W-1:0]       op_ba,
    output    [ROW_W-1:0]       op_a,
    output    [COL_W-1:0]       op_col,
    output     [8*DQ_W-1:0]     op_wdata,
    output       [DQ_W-1:0]     op_wmask,
    input                       op_done,
    input                       op_rd_ok,
    input      [8*DQ_W-1:0]     op_rdata
);

initial begin
    done = 0;
    fail = 0;
    rd_delay = '0;
    wr_shift = 0;
    op_valid = 0;
    op = 0;
end

localparam LANES = DQ_W / 8;
localparam [2:0] OP_RD = 3'd3, OP_WR = 3'd4;
function automatic [31:0] seed(input integer i);
    case (i)
    0: seed = 32'h9a5c_36e1;
    1: seed = 32'h0f1e_2d4b;
    2: seed = 32'h5aa5_c378;
    default: seed = 32'h7e81_18a5;
    endcase
endfunction

function automatic [8*DQ_W-1:0] pattern();
    for (int j = 0; j < 8; j++)
        for (int b = 0; b < DQ_W; b += 16)
            pattern[DQ_W*j + b +: 16] = 16'((j % 2 == 0 ? seed(j / 2) : ~seed(j / 2)) >> (b % 32));
endfunction
localparam [8*DQ_W-1:0] PATTERN = pattern();

assign op_ba = '0;
assign op_a = '0;
assign op_col = '0;
assign op_wdata = PATTERN;
assign op_wmask = '0;

// does each lane match the pattern
wire [LANES-1:0] match;
for (genvar l = 0; l < LANES; l++) begin : g_match
    wire [63:0] got, want;
    for (genvar j = 0; j < 8; j++) begin : g_beat
        assign got[8*j +: 8] = op_rdata[DQ_W*j + 8*l +: 8];
        assign want[8*j +: 8] = PATTERN[DQ_W*j + 8*l +: 8];
    end
    assign match[l] = op_rd_ok && got == want;
end

// window search per lane, plus one for all lanes together
localparam SW = $clog2(STEPS + 1);
reg [SW-1:0] run_start [LANES+1];
reg [SW-1:0] run_len   [LANES+1];
reg [SW-1:0] best_start[LANES+1];
reg [SW-1:0] best_len  [LANES+1];
reg [LANES-1:0] pass;
initial pass = '0;
wire [LANES-1:0] lane_empty;
for (genvar l = 0; l < LANES; l++) begin : g_empty
    assign lane_empty[l] = best_len[l] == 0;
end
wire no_window = PER_LANE ? |lane_empty : best_len[LANES] == 0;

localparam [2:0] S_IDLE = 0, S_WRITE = 1, S_SET = 2, S_READ = 3, S_NEXT = 4, S_FINAL = 5, S_DONE = 6;
reg [2:0] state;
reg [SW-1:0] step;
reg [$clog2(REPEAT+1)-1:0] rep;
reg [1:0] settle;
initial begin
    state = S_IDLE;
    step = 0;
    rep = 0;
    settle = 0;
end

function automatic [SW-1:0] middle(input [SW-1:0] s, input [SW-1:0] len);
    middle = s + SW'((len - 1'd1) >> 1);
endfunction

always @(posedge clk) begin
    if (rst) begin
        state <= S_IDLE;
        done <= 0;
        fail <= 0;
        op_valid <= 0;
        rd_delay <= '0;
        wr_shift <= 0;
    end else if (ce) begin
        case (state)
        S_IDLE: if (start && op_ready) begin
            op_valid <= 1;
            op <= OP_WR;
            state <= S_WRITE;
            step <= 0;
            for (int i = 0; i <= LANES; i++) begin
                run_len[i] <= 0;
                best_len[i] <= 0;
                best_start[i] <= 0;
            end
        end
        S_WRITE: begin
            op_valid <= 0;
            if (op_done) begin
                state <= S_SET;
                rd_delay <= {LANES{DW'(step)}};
                settle <= 0;
            end
        end
        S_SET: begin
            // give the PHY a couple of cycles to drop phy_ready for the new delay
            if (settle != 2'd3) settle <= settle + 1'd1;
            else if (phy_ready && op_ready) begin
                op_valid <= 1;
                op <= OP_RD;
                rep <= 0;
                pass <= '1;
                state <= S_READ;
            end
        end
        S_READ: begin
            op_valid <= 0;
            if (op_done) begin
                if (rep == REPEAT - 1) begin
                    pass <= pass & match;
                    state <= S_NEXT;
                end else begin
                    pass <= pass & match;
                    rep <= rep + 1'd1;
                    op_valid <= 1;
                end
            end
        end
        S_NEXT: begin
            for (int i = 0; i <= LANES; i++) begin
                if (i < LANES ? pass[i] : &pass) begin
                    if (run_len[i] == 0) run_start[i] <= step;
                    run_len[i] <= run_len[i] + 1'd1;
                    if (run_len[i] + 1'd1 > best_len[i]) begin
                        best_len[i] <= run_len[i] + 1'd1;
                        best_start[i] <= run_len[i] == 0 ? step : run_start[i];
                    end
                end else
                    run_len[i] <= 0;
            end
            if (step == SW'(STEPS - 1)) state <= S_FINAL;
            else begin
                step <= step + 1'd1;
                rd_delay <= {LANES{DW'(step + 1'd1)}};
                settle <= 0;
                state <= S_SET;
            end
        end
        S_FINAL: begin
            for (int l = 0; l < LANES; l++)
                rd_delay[DW*l +: DW] <= DW'(PER_LANE ? middle(best_start[l], best_len[l])
                                                     : middle(best_start[LANES], best_len[LANES]));
            settle <= 0;
            if (no_window && wr_shift != 2'(WR_STEPS - 1)) begin
                wr_shift <= wr_shift + 1'd1;
                state <= S_IDLE;
            end else begin
                fail <= no_window;
                state <= S_DONE;
            end
        end
        default: begin
            if (settle != 2'd3) settle <= settle + 1'd1;
            else if (phy_ready) done <= 1;
        end
        endcase
    end
end

endmodule

`endif /* module_ddr3_calib */
