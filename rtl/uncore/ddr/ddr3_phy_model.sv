`ifndef module_ddr3_phy_model
`define module_ddr3_phy_model

// Simulation stand-in for ddr3_phy_slow and the DDR3 chip behind it (gowin-ddr3).
//
// It has the controller side of ddr3_phy_slow's ports, the same ce (every other clk
// edge) and the same read latency, so ddr3_ctrl runs unchanged on top of it. Data
// lives in a sparse array indexed by the controller's word address {bank, row, column}.
//
// Reads model where the DRAM's beats fall among the PHY's half-clk samples (numbered
// as in ddr3_phy_slow): beat j of lane l is on DQ for samples H0 + RD_SKEW[l] + 2j and
// the one after. A lane read with delay d therefore gets beat j + floor((d - skew) / 2),
// or the idle bus outside the burst, so read calibration has a real window to find:
// delays RD_SKEW[l] and RD_SKEW[l] + 1.
//
// Checks ($error): ACTIVATE to a precharged bank only, READ/WRITE to an activated bank
// only and BL8, REFRESH with all banks precharged, and with REFI_MAX > 0 no more than
// REFI_MAX controller cycles between refreshes once the first one after a DRAM reset was
// issued.
module ddr3_phy_model #(
    parameter DQ_W  = 32,
    parameter ROW_W = 15,
    parameter COL_W = 10,
    parameter BA_W  = 3,
    parameter CL    = 6,
    parameter DW    = 3,                                // read delay bits per lane
    parameter [DQ_W/8*DW-1:0] RD_SKEW = {(DQ_W/8){DW'(1)}},
    parameter REFI_MAX = 0,                             // controller cycles, 0: no check
    localparam LANES = DQ_W / 8,
    localparam STEPS = 2 ** DW,
    localparam BANKS = 2 ** BA_W
) (
    input                       clk,
    output reg                  ce,

    input                       phy_reset_n,
    input                       phy_cke,
    input                 [2:0] phy_cmd,
    input           [ROW_W-1:0] phy_addr,
    input            [BA_W-1:0] phy_ba,
    input                       phy_wr,
    input          [8*DQ_W-1:0] phy_wdata,
    input            [DQ_W-1:0] phy_wmask,
    input                       phy_rd,
    input      [LANES*DW-1:0]   phy_rd_delay,
    output reg                  phy_rvalid,
    output reg   [8*DQ_W-1:0]   phy_rdata,
    output                      phy_ready,

    // not modelled, present for port compatibility with ddr3_phy_slow
    /* verilator lint_off UNUSEDSIGNAL */
    input                       phy_odt,
    input                 [1:0] phy_wr_ph,
    input                 [1:0] phy_rd_ph,
    input                 [1:0] phy_wr_shift
    /* verilator lint_on UNUSEDSIGNAL */
);

localparam [2:0] CMD_REF = 3'b001, CMD_PRE = 3'b010, CMD_ACT = 3'b011,
                 CMD_WR  = 3'b100, CMD_RD  = 3'b101;
localparam AW = BA_W + ROW_W + COL_W;

initial begin
    ce = 0;
    phy_rvalid = 0;
    phy_rdata = '0;
end
always @(posedge clk) ce <= ~ce;
assign phy_ready = 1'b1;

logic [DQ_W-1:0] mem [bit [AW-1:0]];                // sparse: only written words exist

function automatic [DQ_W-1:0] peek(input [AW-1:0] a);
    peek = mem.exists(a) != 0 ? mem[a] : '0;
endfunction

// Bank state. Commands are taken on the clk edge where CK rises (ce low), as the DRAM
// samples them; the controller changes them on the ce edges.
reg [BANKS-1:0] open;
reg [ROW_W-1:0] open_row [BANKS];
reg  [AW-1:0]   wr_base, rd_base;                     // burst start {bank, row, column}
initial open = '0;

wire           cmd_valid = !ce && phy_reset_n && phy_cke;
wire [COL_W-4:0] col = phy_addr[COL_W-1:3];        // burst: columns col*8 .. col*8+7
wire           auto_pre = phy_addr[10];

always @(posedge clk) begin
    if (cmd_valid) begin
        case (phy_cmd)
        CMD_ACT: begin
            if (open[phy_ba]) $error("ddr3_phy_model: ACTIVATE to open bank %0d", phy_ba);
            open[phy_ba] <= 1;
            open_row[phy_ba] <= phy_addr;
        end
        CMD_RD, CMD_WR: begin
            if (!open[phy_ba]) $error("ddr3_phy_model: READ/WRITE to closed bank %0d", phy_ba);
            if (ROW_W > 12 && !phy_addr[12]) $error("ddr3_phy_model: only BL8 is modelled");
            if (phy_cmd == CMD_RD) rd_base <= {phy_ba, open_row[phy_ba], col, 3'b000};
            else                   wr_base <= {phy_ba, open_row[phy_ba], col, 3'b000};
            if (auto_pre) open[phy_ba] <= 0;
        end
        CMD_PRE: begin
            if (auto_pre) open <= '0;
            else open[phy_ba] <= 0;
        end
        CMD_REF: if (|open) $error("ddr3_phy_model: REFRESH with open banks %b", open);
        default: ;                                      // MRS, ZQCL, NOP
        endcase
    end
end

// Writes: the PHY takes the data on the ce edge after the WRITE command
always @(posedge clk) begin
    if (ce && phy_wr)
        for (int j = 0; j < 8; j++) begin
            logic [DQ_W-1:0] w;
            w = peek(wr_base + AW'(j));
            for (int l = 0; l < LANES; l++)
                if (!phy_wmask[LANES*j + l]) w[8*l +: 8] = phy_wdata[DQ_W*j + 8*l +: 8];
            // blocking: a nonblocking write to an associative array makes Verilator copy the
            // whole array on every write. Safe, as no read of the same burst completes on this edge.
            /* verilator lint_off BLKSEQ */
            mem[wr_base + AW'(j)] = w;
            /* verilator lint_on BLKSEQ */
        end
end

// Reads: same latency as ddr3_phy_slow (counted from the ce edge after the READ command)
localparam integer H0 = 4 * (CL - 1) - 2;
localparam integer U_END = (H0 + STEPS - 1 + 14) / 2 + 2;
localparam UW = $clog2(U_END + 2);

reg          r_act;
reg [UW-1:0] ru;
initial begin
    r_act = 0;
    ru = 0;
end

function automatic [8*DQ_W-1:0] read_burst(input [AW-1:0] base, input [LANES*DW-1:0] delay);
    read_burst = '0;
    for (int l = 0; l < LANES; l++) begin
        integer o, k;
        o = int'(delay[DW*l +: DW]) - int'(RD_SKEW[DW*l +: DW]);
        for (int j = 0; j < 8; j++) begin
            k = j + (o >= 0 ? o / 2 : -((1 - o) / 2));   // floor(o / 2)
            read_burst[DQ_W*j + 8*l +: 8] = k >= 0 && k < 8 ? peek(base + AW'(k))[8*l +: 8] : 8'hff;
        end
    end
endfunction

always @(posedge clk) begin
    if (ce && phy_rd) begin
        r_act <= 1;
        ru <= 1;
    end else if (r_act) begin
        ru <= ru + 1'd1;
        if (ru == UW'(U_END)) begin
            r_act <= 0;
            phy_rvalid <= 1;
            phy_rdata <= read_burst(rd_base, phy_rd_delay);
        end
    end
    if (phy_rvalid && ce) phy_rvalid <= 0;
end

// Refresh interval, in controller cycles
generate if (REFI_MAX > 0) begin : g_refi
    integer since;
    reg     started;
    initial begin
        since = 0;
        started = 0;
    end
    always @(posedge clk) begin
        if (!phy_reset_n)                               // DRAM reset: start over
            started <= 0;
        else if (cmd_valid && phy_cmd == CMD_REF) begin
            started <= 1;
            since <= 0;
        end else if (ce && started) begin
            since <= since + 1;
            if (since == REFI_MAX) $error("ddr3_phy_model: no REFRESH for %0d controller cycles", REFI_MAX);
        end
    end
end endgenerate

endmodule

`endif /* module_ddr3_phy_model */
