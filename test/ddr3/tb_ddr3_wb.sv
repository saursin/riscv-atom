// cocotb top: DDR3_wb (Wishbone adapter + gowin-ddr3 controller) on ddr3_phy_model, with the
// TangPrimer 20K parameters. The model checks the refresh interval: at most 2 x tREFI.
`include "ddr3_phy_model.sv"
`include "DDR3_wb.v"

module tb_ddr3_wb #(
    parameter DQ_W = 16, ROW_W = 13, COL_W = 10, BA_W = 3, DW = 3,
    parameter CLK_FREQ = 45_000_000,
    parameter [DQ_W/8*DW-1:0] RD_SKEW = {3'd2, 3'd1},     // lane 1, lane 0
    localparam LANES = DQ_W / 8, MEM_AW = BA_W + ROW_W + COL_W + $clog2(LANES),
    localparam REFI_CK = CLK_FREQ / 1000 * 78 / 10000 / 2  // tREFI in controller cycles
) (
    input                     clk,
    input                     rst,

    input        [MEM_AW-1:2] mem_adr,
    output              [31:0] mem_dat_o,
    input               [31:0] mem_dat_i,
    input                     mem_we,
    input                [3:0] mem_sel,
    input                     mem_stb,
    output                    mem_ack,

    input                [3:2] ctrl_adr,
    output              [31:0] ctrl_dat_o,
    input                     ctrl_stb,
    output                    ctrl_ack
);

wire              ce, phy_reset_n, phy_cke, phy_odt, phy_wr, phy_rd, phy_rvalid, phy_ready;
wire        [2:0] phy_cmd;
wire  [ROW_W-1:0] phy_addr;
wire   [BA_W-1:0] phy_ba;
wire        [1:0] phy_wr_ph, phy_rd_ph, phy_wr_shift;
wire [8*DQ_W-1:0] phy_wdata, phy_rdata;
wire   [DQ_W-1:0] phy_wmask;
wire [LANES*DW-1:0] phy_rd_delay;

DDR3_wb #(
    .CLK_FREQ(CLK_FREQ), .DQ_W(DQ_W), .ROW_W(ROW_W), .COL_W(COL_W), .BA_W(BA_W),
    .CAL_DW(DW), .SIM(1)
) ddr (
    .wb_clk_i(clk), .wb_rst_i(rst),
    .mem_wb_adr_i(mem_adr), .mem_wb_dat_o(mem_dat_o), .mem_wb_dat_i(mem_dat_i),
    .mem_wb_we_i(mem_we), .mem_wb_sel_i(mem_sel), .mem_wb_stb_i(mem_stb), .mem_wb_ack_o(mem_ack),
    .ctrl_wb_adr_i(ctrl_adr), .ctrl_wb_dat_o(ctrl_dat_o), .ctrl_wb_dat_i(32'd0),
    .ctrl_wb_we_i(1'b0), .ctrl_wb_sel_i(4'd0), .ctrl_wb_stb_i(ctrl_stb), .ctrl_wb_ack_o(ctrl_ack),
    .phy_ce(ce), .phy_reset_n(phy_reset_n), .phy_cke(phy_cke), .phy_odt(phy_odt),
    .phy_cmd(phy_cmd), .phy_addr(phy_addr), .phy_ba(phy_ba),
    .phy_wr(phy_wr), .phy_wr_ph(phy_wr_ph), .phy_wdata(phy_wdata), .phy_wmask(phy_wmask),
    .phy_rd(phy_rd), .phy_rd_ph(phy_rd_ph), .phy_rd_delay(phy_rd_delay), .phy_wr_shift(phy_wr_shift),
    .phy_rvalid(phy_rvalid), .phy_rdata(phy_rdata), .phy_ready(phy_ready)
);

ddr3_phy_model #(
    .DQ_W(DQ_W), .ROW_W(ROW_W), .COL_W(COL_W), .BA_W(BA_W), .CL(6), .DW(DW), .RD_SKEW(RD_SKEW),
    .REFI_MAX(2 * REFI_CK)
) phy (
    .clk(clk), .ce(ce),
    .phy_reset_n(phy_reset_n), .phy_cke(phy_cke), .phy_cmd(phy_cmd), .phy_addr(phy_addr),
    .phy_ba(phy_ba), .phy_wr(phy_wr), .phy_wdata(phy_wdata), .phy_wmask(phy_wmask),
    .phy_rd(phy_rd), .phy_rd_delay(phy_rd_delay), .phy_rvalid(phy_rvalid), .phy_rdata(phy_rdata),
    .phy_ready(phy_ready),
    .phy_odt(phy_odt), .phy_wr_ph(phy_wr_ph), .phy_rd_ph(phy_rd_ph), .phy_wr_shift(phy_wr_shift)
);

endmodule
