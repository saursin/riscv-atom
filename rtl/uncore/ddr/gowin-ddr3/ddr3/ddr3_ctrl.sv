`ifndef module_ddr3_ctrl
`define module_ddr3_ctrl

`include "ddr3/ddr3_cmd.sv"
`include "ddr3/ddr3_init.sv"
`include "ddr3/ddr3_calib.sv"

// Simple low-latency DDR3 controller, PHY independent.
//
// Derived from nand2mario's DDR3 controller for the Tang Primer 20K
// (https://github.com/nand2mario/ddr3-tang-primer-20k, Apache-2.0): single
// word accesses, closed page, refresh requested by the user so that the
// latency stays predictable.
//
// Usage: while busy is low, pulse rd, wr or refresh for one clk cycle with
// addr (and din/be). busy rises in the next cycle and drops when the access
// is done; for reads data_ready pulses together with dout/dout_burst.
// Refresh is up to the user: one every 7.8us (tREFI) on average.
//
// The PHY decides the clocking: the controller works on its ce cycles, each of
// which covers NPH DRAM clocks, see ddr3_phy_slow and ddr3_phy_2to1.
module ddr3_ctrl #(
    parameter DQ_W   = 32,
    parameter ROW_W  = 15,
    parameter COL_W  = 10,
    parameter BA_W   = 3,
    parameter NPH    = 1,
    parameter TCK_PS = 40000,
    parameter CL     = 6,
    parameter CWL    = 6,
    parameter NWR    = 5,
    parameter DLL_OFF = 1,
    parameter [1:0] RTT_WR = 2'b00,
    parameter T_RFC_PS = 260000,        // 4Gb devices
    // read calibration, given by the PHY
    parameter CAL_DW    = 3,
    parameter CAL_STEPS = 8,
    parameter CAL_PER_LANE = 1,
    parameter CAL_WR_STEPS = 1,
    parameter SIM    = 0,
    localparam ADDR_W = BA_W + ROW_W + COL_W,
    localparam LANES = DQ_W / 8
) (
    input                       clk,
    input                       ce,
    input                       rst,

    // user side, addr is a DQ_W bit word address {bank, row, column}
    input                       rd,
    input                       wr,
    input                       refresh,
    input          [ADDR_W-1:0] addr,
    input            [DQ_W-1:0] din,
    input           [LANES-1:0] be,             // byte enables for wr
    output reg       [DQ_W-1:0] dout,
    output reg     [8*DQ_W-1:0] dout_burst, // the whole aligned BL8 burst
    output reg                  data_ready,
    output reg                  busy,
    output                      init_done,
    output                      calib_done,
    output                      calib_fail,

    // PHY
    output                      phy_reset_n,
    output                      phy_cke,
    output                      phy_odt,
    output          [NPH*3-1:0] phy_cmd,
    output      [NPH*ROW_W-1:0] phy_addr,
    output       [NPH*BA_W-1:0] phy_ba,
    output                      phy_wr,
    output                [1:0] phy_wr_ph,
    output       [8*DQ_W-1:0]   phy_wdata,
    output         [DQ_W-1:0]   phy_wmask,
    output                      phy_rd,
    output                [1:0] phy_rd_ph,
    output  [LANES*CAL_DW-1:0]  phy_rd_delay,
    output                [1:0] phy_wr_shift,
    input                       phy_rvalid,
    input        [8*DQ_W-1:0]   phy_rdata,
    input                       phy_ready
);

initial begin
    dout = '0;
    dout_burst = '0;
    data_ready = 0;
    busy = 1;
end

localparam [2:0] OP_REF = 3'd2, OP_RD = 3'd3, OP_WR = 3'd4;

// operation port, shared by init, calibration and the user
wire              op_valid, op_ready, op_done, op_rd_ok;
wire        [2:0] op;
wire   [BA_W-1:0] op_ba;
wire  [ROW_W-1:0] op_a;
wire  [COL_W-1:0] op_col;
wire [8*DQ_W-1:0] op_wdata, op_rdata;
wire   [DQ_W-1:0] op_wmask;

wire              i_valid;
wire        [2:0] i_op;
wire   [BA_W-1:0] i_ba;
wire  [ROW_W-1:0] i_a;

wire              c_valid;
wire        [2:0] c_op;
wire   [BA_W-1:0] c_ba;
wire  [ROW_W-1:0] c_a;
wire  [COL_W-1:0] c_col;
wire [8*DQ_W-1:0] c_wdata;
wire   [DQ_W-1:0] c_wmask;

reg               u_valid;
reg         [2:0] u_op;
reg  [ADDR_W-1:0] u_addr;
reg    [DQ_W-1:0] u_din;
reg   [LANES-1:0] u_be;
initial begin
    u_valid = 0;
    u_op = 0;
    u_addr = 0;
    u_din = 0;
    u_be = 0;
end

wire in_init = !init_done;
wire in_calib = init_done && !calib_done;

assign op_valid = in_init ? i_valid : in_calib ? c_valid : u_valid;
assign op       = in_init ? i_op    : in_calib ? c_op    : u_op;
assign op_ba    = in_init ? i_ba    : in_calib ? c_ba    : u_addr[ADDR_W-1 -: BA_W];
assign op_a     = in_init ? i_a     : in_calib ? c_a     : u_addr[COL_W +: ROW_W];
assign op_col   = in_calib ? c_col : u_addr[COL_W-1:0];
assign op_wdata = in_calib ? c_wdata : {8{u_din}};

wire [2:0] u_beat = u_addr[2:0];
for (genvar j = 0; j < 8; j++) begin : g_mask
    assign op_wmask[LANES*j +: LANES] = in_calib ? c_wmask[LANES*j +: LANES]
                                      : 3'(j) == u_beat ? ~u_be : '1;
end

ddr3_cmd #(
    .NPH(NPH), .DQ_W(DQ_W), .ROW_W(ROW_W), .COL_W(COL_W), .BA_W(BA_W),
    .TCK_PS(TCK_PS), .CWL(CWL), .NWR(NWR), .T_RFC_PS(T_RFC_PS)
) cmd (
    .clk(clk), .ce(ce), .rst(rst),
    .op_valid(op_valid), .op_ready(op_ready), .op(op), .op_ba(op_ba), .op_a(op_a),
    .op_col(op_col), .op_wdata(op_wdata), .op_wmask(op_wmask),
    .op_done(op_done), .op_rd_ok(op_rd_ok), .op_rdata(op_rdata),
    .phy_cmd(phy_cmd), .phy_addr(phy_addr), .phy_ba(phy_ba),
    .phy_wr(phy_wr), .phy_wr_ph(phy_wr_ph), .phy_wdata(phy_wdata), .phy_wmask(phy_wmask),
    .phy_rd(phy_rd), .phy_rd_ph(phy_rd_ph), .phy_rvalid(phy_rvalid), .phy_rdata(phy_rdata)
);

ddr3_init #(
    .ROW_W(ROW_W), .BA_W(BA_W), .CTL_PS(NPH * TCK_PS), .T_XPR_PS(T_RFC_PS + 10000),
    .CL(CL), .CWL(CWL), .NWR(NWR), .DLL_OFF(DLL_OFF), .RTT_WR(RTT_WR), .SIM(SIM)
) init (
    .clk(clk), .ce(ce), .rst(rst),
    .reset_n(phy_reset_n), .cke(phy_cke), .done(init_done),
    .op_valid(i_valid), .op_ready(op_ready), .op(i_op), .op_ba(i_ba), .op_a(i_a),
    .op_done(op_done)
);

ddr3_calib #(
    .DQ_W(DQ_W), .ROW_W(ROW_W), .COL_W(COL_W), .BA_W(BA_W),
    .DW(CAL_DW), .STEPS(CAL_STEPS), .PER_LANE(CAL_PER_LANE), .WR_STEPS(CAL_WR_STEPS)
) calib (
    .clk(clk), .ce(ce), .rst(rst), .start(init_done),
    .done(calib_done), .fail(calib_fail), .rd_delay(phy_rd_delay), .wr_shift(phy_wr_shift),
    .phy_ready(phy_ready),
    .op_valid(c_valid), .op_ready(op_ready), .op(c_op), .op_ba(c_ba), .op_a(c_a),
    .op_col(c_col), .op_wdata(c_wdata), .op_wmask(c_wmask),
    .op_done(op_done), .op_rd_ok(op_rd_ok), .op_rdata(op_rdata)
);

// dynamic ODT: with RTT_NOM off, ODT high only enables RTT_WR during writes
assign phy_odt = RTT_WR != 2'b00 && init_done;

// user requests, accepted in any clk cycle, executed in ce cycles
reg req;
reg pending;
initial begin
    req = 0;
    pending = 0;
end
always @(posedge clk) begin
    data_ready <= 0;
    if (rst) begin
        busy <= 1;
        req <= 0;
        pending <= 0;
        u_valid <= 0;
    end else begin
        if (!busy && (rd || wr || refresh)) begin
            busy <= 1;
            req <= 1;
            u_op <= rd ? OP_RD : wr ? OP_WR : OP_REF;
            u_addr <= addr;
            u_din <= din;
            u_be <= be;
        end
        if (ce) begin
            if (calib_done && !req && !pending && busy && !u_valid) busy <= 0;  // after init/calibration
            if (req && calib_done && op_ready && !u_valid) begin
                u_valid <= 1;
                req <= 0;
                pending <= 1;
            end
            if (u_valid) u_valid <= 0;
            if (pending && !u_valid && op_done) begin
                pending <= 0;
                busy <= 0;
                if (u_op == OP_RD) begin
                    data_ready <= 1;
                    dout_burst <= op_rdata;
                    dout <= op_rdata[DQ_W*u_beat +: DQ_W];
                end
            end
        end
    end
end

endmodule

`endif /* module_ddr3_ctrl */
