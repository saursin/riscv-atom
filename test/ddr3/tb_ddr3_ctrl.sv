// cocotb top: the vendored gowin-ddr3 controller on ddr3_phy_model, with the TangPrimer 20K
// parameters (16-bit 1Gb chip, 45 MHz controller clock, CK = 22.5 MHz). The model's read
// skew differs per lane, so calibration must find a window for each lane.
`include "ddr3/ddr3_ctrl.sv"
`include "ddr3_phy_model.sv"

module tb_ddr3_ctrl #(
    parameter DQ_W = 16, ROW_W = 13, COL_W = 10, BA_W = 3, DW = 3,
    parameter SYS_FREQ = 45_000_000,
    parameter [DQ_W/8*DW-1:0] RD_SKEW = {3'd2, 3'd1},     // lane 1, lane 0
    localparam ADDR_W = BA_W + ROW_W + COL_W, LANES = DQ_W / 8
) (
    input                     clk,
    input                     rst,
    input                     rd,
    input                     wr,
    input                     refresh,
    input        [ADDR_W-1:0] addr,
    input          [DQ_W-1:0] din,
    input         [LANES-1:0] be,
    output         [DQ_W-1:0] dout,
    output       [8*DQ_W-1:0] dout_burst,
    output                    data_ready,
    output                    busy,
    output                    init_done,
    output                    calib_done,
    output                    calib_fail,
    output     [LANES*DW-1:0] rd_delay
);

wire              ce, phy_reset_n, phy_cke, phy_odt, phy_wr, phy_rd, phy_rvalid, phy_ready;
wire        [2:0] phy_cmd;
wire  [ROW_W-1:0] phy_addr;
wire   [BA_W-1:0] phy_ba;
wire        [1:0] phy_wr_ph, phy_rd_ph, phy_wr_shift;
wire [8*DQ_W-1:0] phy_wdata, phy_rdata;
wire   [DQ_W-1:0] phy_wmask;

ddr3_ctrl #(
    .DQ_W(DQ_W), .ROW_W(ROW_W), .COL_W(COL_W), .BA_W(BA_W),
    .NPH(1), .TCK_PS(2_000_000_000 / (SYS_FREQ / 1000)), .CL(6), .CWL(6), .NWR(5), .DLL_OFF(1),
    .T_RFC_PS(110_000), .CAL_DW(DW), .CAL_STEPS(2 ** DW), .CAL_PER_LANE(1), .SIM(1)
) ctrl (
    .clk(clk), .ce(ce), .rst(rst),
    .rd(rd), .wr(wr), .refresh(refresh), .addr(addr), .din(din), .be(be),
    .dout(dout), .dout_burst(dout_burst), .data_ready(data_ready), .busy(busy),
    .init_done(init_done), .calib_done(calib_done), .calib_fail(calib_fail),
    .phy_reset_n(phy_reset_n), .phy_cke(phy_cke), .phy_odt(phy_odt),
    .phy_cmd(phy_cmd), .phy_addr(phy_addr), .phy_ba(phy_ba),
    .phy_wr(phy_wr), .phy_wr_ph(phy_wr_ph), .phy_wdata(phy_wdata), .phy_wmask(phy_wmask),
    .phy_rd(phy_rd), .phy_rd_ph(phy_rd_ph), .phy_rd_delay(rd_delay), .phy_wr_shift(phy_wr_shift),
    .phy_rvalid(phy_rvalid), .phy_rdata(phy_rdata), .phy_ready(phy_ready)
);

ddr3_phy_model #(
    .DQ_W(DQ_W), .ROW_W(ROW_W), .COL_W(COL_W), .BA_W(BA_W), .CL(6), .DW(DW), .RD_SKEW(RD_SKEW)
) phy (
    .clk(clk), .ce(ce),
    .phy_reset_n(phy_reset_n), .phy_cke(phy_cke), .phy_cmd(phy_cmd), .phy_addr(phy_addr),
    .phy_ba(phy_ba), .phy_wr(phy_wr), .phy_wdata(phy_wdata), .phy_wmask(phy_wmask),
    .phy_rd(phy_rd), .phy_rd_delay(rd_delay), .phy_rvalid(phy_rvalid), .phy_rdata(phy_rdata),
    .phy_ready(phy_ready),
    .phy_odt(phy_odt), .phy_wr_ph(phy_wr_ph), .phy_rd_ph(phy_rd_ph), .phy_wr_shift(phy_wr_shift)
);

endmodule
