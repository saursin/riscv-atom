`ifndef module_ddr3_cmd
`define module_ddr3_cmd

// DDR3 command sequencer.
//
// Executes one operation at a time and keeps the DRAM timing: mode register
// set, ZQ calibration, refresh, and closed-page reads/writes (ACTIVATE, then
// READ/WRITE with auto-precharge, BL8 from column 0).
//
// The controller runs at 1/NPH of the DRAM clock: every enabled (ce) cycle
// covers NPH DRAM clocks, so each cycle carries NPH command slots, slot 0
// being the earliest. Write data and read requests go to the PHY together with
// the slot of their WRITE/READ command.
module ddr3_cmd #(
    parameter NPH    = 1,               // DRAM clocks per controller cycle
    parameter DQ_W   = 32,
    parameter ROW_W  = 15,
    parameter COL_W  = 10,
    parameter BA_W   = 3,
    parameter TCK_PS = 2500,
    parameter CWL    = 5,
    parameter NWR    = 6,               // write recovery as programmed in MR0
    parameter T_RCD_PS = 15000,
    parameter T_RP_PS  = 15000,
    parameter T_RAS_PS = 37500,
    parameter T_RFC_PS = 260000,
    parameter T_RTP_PS = 7500,
    parameter T_MOD_PS = 15000,
    parameter N_ZQINIT = 512,           // ZQCL (and DLL lock) time in DRAM clocks
    parameter RD_TIMEOUT = 64           // controller cycles to wait for read data
) (
    input                       clk,
    input                       ce,
    input                       rst,

    // operation
    input                       op_valid,
    output                      op_ready,
    input                 [2:0] op,
    input            [BA_W-1:0] op_ba,
    input           [ROW_W-1:0] op_a,       // row, or the MRS/ZQCL address bits
    input           [COL_W-1:0] op_col,
    input        [8*DQ_W-1:0]   op_wdata,   // 8 beats, beat 0 in the low bits
    input        [DQ_W-1:0]     op_wmask,   // 8 beats x DQ_W/8 byte lanes, 1 masks
    output reg                  op_done,
    output reg                  op_rd_ok,
    output reg   [8*DQ_W-1:0]   op_rdata,

    // PHY
    output reg      [NPH*3-1:0] phy_cmd,     // {RAS#, CAS#, WE#} per slot
    output reg  [NPH*ROW_W-1:0] phy_addr,
    output reg   [NPH*BA_W-1:0] phy_ba,
    output reg                  phy_wr,
    output reg            [1:0] phy_wr_ph,
    output reg   [8*DQ_W-1:0]   phy_wdata,
    output reg     [DQ_W-1:0]   phy_wmask,
    output reg                  phy_rd,
    output reg            [1:0] phy_rd_ph,
    input                       phy_rvalid,
    input        [8*DQ_W-1:0]   phy_rdata
);

initial begin
    op_done = 0;
    op_rd_ok = 0;
    op_rdata = '0;
    phy_cmd = {NPH{3'b111}};
    phy_addr = '0;
    phy_ba = '0;
    phy_wr = 0;
    phy_wr_ph = 0;
    phy_wdata = '0;
    phy_wmask = '1;
    phy_rd = 0;
    phy_rd_ph = 0;
end

localparam [2:0] OP_MRS = 3'd0, OP_ZQCL = 3'd1, OP_REF = 3'd2, OP_RD = 3'd3, OP_WR = 3'd4;
localparam [2:0] CMD_MRS = 3'b000, CMD_REF = 3'b001, CMD_ACT = 3'b011,
                 CMD_WR  = 3'b100, CMD_RD  = 3'b101, CMD_ZQ  = 3'b110, CMD_NOP = 3'b111;

function automatic integer nck(input integer ps, input integer min_ck);
    nck = (ps + TCK_PS - 1) / TCK_PS;
    if (nck < min_ck) nck = min_ck;
endfunction
function automatic integer max2(input integer a, input integer b);
    max2 = a > b ? a : b;
endfunction
function automatic integer cycles(input integer ck);     // DRAM clocks -> controller cycles
    cycles = (ck + NPH - 1) / NPH;
endfunction

localparam integer N_RCD = nck(T_RCD_PS, 1);
localparam integer N_RP  = nck(T_RP_PS, 1);
localparam integer N_RAS = nck(T_RAS_PS, 1);
localparam integer N_RFC = nck(T_RFC_PS, 1);
localparam integer N_RTP = nck(T_RTP_PS, 4);
localparam integer N_MOD = nck(T_MOD_PS, 12);

// clocks from ACTIVATE until the bank is precharged again
localparam integer N_RD_END = max2(N_RCD + N_RTP, N_RAS) + N_RP;
localparam integer N_WR_END = max2(N_RCD + CWL + 4 + NWR, N_RAS) + N_RP;

localparam integer RW_CYC  = N_RCD / NPH;               // cycle and slot of READ/WRITE
localparam integer RW_SLOT = N_RCD % NPH;
localparam integer W = 12;
localparam [W-1:0] END_MRS = W'(cycles(N_MOD));
localparam [W-1:0] END_ZQ  = W'(cycles(N_ZQINIT));
localparam [W-1:0] END_REF = W'(cycles(N_RFC));
localparam [W-1:0] END_RD  = W'(cycles(N_RD_END));
localparam [W-1:0] END_WR  = W'(cycles(N_WR_END));

initial begin
    if (RW_CYC < 1) $error("ddr3_cmd: tRCD shorter than one controller cycle is not supported");
    if (RD_TIMEOUT >= 2**W) $error("ddr3_cmd: RD_TIMEOUT too large");
end

reg           busy;
reg     [2:0] cur_op;
reg   [W-1:0] cyc, cyc_end;
reg           rd_wait;
reg  [BA_W-1:0]  cur_ba;
reg  [COL_W-1:0] cur_col;
reg [8*DQ_W-1:0] cur_wdata;
reg   [DQ_W-1:0] cur_wmask;
initial begin
    busy = 0;
    cur_op = 0;
    cyc = 0;
    cyc_end = 0;
    rd_wait = 0;
    cur_ba = 0;
    cur_col = 0;
    cur_wdata = 0;
    cur_wmask = 0;
end

assign op_ready = !busy;

// column address bits: A10 auto precharge, A12 BL8
function automatic [ROW_W-1:0] col_addr(input [COL_W-1:0] col);
    col_addr = '0;
    col_addr[COL_W-1:0] = {col[COL_W-1:3], 3'b000};
    col_addr[10] = 1'b1;
    col_addr[12] = 1'b1;
endfunction

always @(posedge clk) begin
    if (rst) begin
        busy <= 0;
        op_done <= 0;
        phy_cmd <= {NPH{CMD_NOP}};
        phy_wr <= 0;
        phy_rd <= 0;
        rd_wait <= 0;
    end else if (ce) begin
        phy_cmd <= {NPH{CMD_NOP}};
        phy_wr <= 0;
        phy_rd <= 0;
        op_done <= 0;
        if (phy_rvalid && rd_wait) begin
            rd_wait <= 0;
            op_rd_ok <= 1;
            op_rdata <= phy_rdata;
        end
        if (!busy) begin
            if (op_valid) begin
                busy <= 1;
                cur_op <= op;
                cur_ba <= op_ba;
                cur_col <= op_col;
                cur_wdata <= op_wdata;
                cur_wmask <= op_wmask;
                cyc <= 0;
                phy_ba[0 +: BA_W] <= op_ba;
                phy_addr[0 +: ROW_W] <= op_a;
                case (op)
                OP_MRS:  begin phy_cmd[2:0] <= CMD_MRS; cyc_end <= END_MRS; end
                OP_ZQCL: begin phy_cmd[2:0] <= CMD_ZQ;  cyc_end <= END_ZQ;  end
                OP_REF:  begin phy_cmd[2:0] <= CMD_REF; cyc_end <= END_REF; end
                OP_RD:   begin phy_cmd[2:0] <= CMD_ACT; cyc_end <= END_RD;  end
                default: begin phy_cmd[2:0] <= CMD_ACT; cyc_end <= END_WR;  end
                endcase
                op_rd_ok <= 0;
            end
        end else begin
            cyc <= cyc + 1'd1;
            if (cyc + 1'd1 == W'(RW_CYC) && (cur_op == OP_RD || cur_op == OP_WR)) begin
                phy_cmd[3*RW_SLOT +: 3] <= cur_op == OP_RD ? CMD_RD : CMD_WR;
                phy_ba[BA_W*RW_SLOT +: BA_W] <= cur_ba;
                phy_addr[ROW_W*RW_SLOT +: ROW_W] <= col_addr(cur_col);
                if (cur_op == OP_RD) begin
                    phy_rd <= 1;
                    phy_rd_ph <= 2'(RW_SLOT);
                    rd_wait <= 1;
                end else begin
                    phy_wr <= 1;
                    phy_wr_ph <= 2'(RW_SLOT);
                    phy_wdata <= cur_wdata;
                    phy_wmask <= cur_wmask;
                end
            end
            // a read finishes when the data is back (or timed out) and the bank is precharged
            if (cyc + 1'd1 >= cyc_end && !(cur_op == OP_RD && rd_wait && !phy_rvalid && cyc < W'(RD_TIMEOUT))) begin
                busy <= 0;
                op_done <= 1;
                rd_wait <= 0;
            end
        end
    end
end

endmodule

`endif /* module_ddr3_cmd */
