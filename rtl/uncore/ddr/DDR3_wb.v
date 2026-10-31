////////////////////////////////////////////////////////////////////
//  File        : DDR3_wb.v
//  Description : DDR3 memory with Wishbone interfaces, on the gowin-ddr3 controller
//                (rtl/uncore/ddr/gowin-ddr3). The PHY is outside, in the board shell
//                (ddr3_phy_slow on hardware, ddr3_phy_model in simulation).
//
//  Memory port: plain 32-bit memory.
//    read : one controller read; the 32 bits come out of the burst it returns
//    write: one controller write per DQ_W-bit half with a byte enabled (DQ_W=16: 1 or 2)
//    Accesses wait while the controller initialises and calibrates (about 0.7 ms).
//    After a failed calibration they still complete, with unreliable data.
//  Register port (read-only, writes ignored):
//    0x0 STATUS  bit 0 READY (calibrated), bit 1 CALIB_FAIL
//    0x4 CALIB   read delay per byte lane, CAL_DW bits each, lane 0 lowest
//  Refresh: requested every 7.8 us, between accesses.
////////////////////////////////////////////////////////////////////
`default_nettype none
`include "Utils.vh"

// The vendored controller (rtl/uncore/ddr/gowin-ddr3 on the include path), waiving its one
// -Wall warning: ddr3_cmd ignores the low column bits (bursts start on 8-word boundaries)
/* verilator lint_off UNUSEDSIGNAL */
`include "ddr3/ddr3_ctrl.sv"
/* verilator lint_on UNUSEDSIGNAL */

module DDR3_wb #(
    parameter CLK_FREQ  = 45_000_000,           // Hz; CK = CLK_FREQ / 2
    parameter DQ_W      = 16,                   // 16 or 32
    parameter ROW_W     = 13,
    parameter COL_W     = 10,
    parameter BA_W      = 3,
    parameter T_RFC_PS  = 110_000,              // 1Gb parts
    parameter CAL_DW    = 3,
    parameter SIM       = 0,                    // short power-up waits, simulation only
    localparam LANES    = DQ_W / 8,
    localparam DRAM_AW  = BA_W + ROW_W + COL_W, // DQ_W-bit word address
    localparam MEM_AW   = DRAM_AW + $clog2(LANES)
)(
    input   wire                    wb_clk_i,
    input   wire                    wb_rst_i,

    // Memory
    input   wire    [MEM_AW-1:2]    mem_wb_adr_i,
    output  reg     [31:0]          mem_wb_dat_o,
    input   wire    [31:0]          mem_wb_dat_i,
    input   wire                    mem_wb_we_i,
    input   wire    [3:0]           mem_wb_sel_i,
    input   wire                    mem_wb_stb_i,
    output  reg                     mem_wb_ack_o,

    // Registers
    input   wire    [3:2]           ctrl_wb_adr_i,
    output  reg     [31:0]          ctrl_wb_dat_o,
    input   wire    [31:0]          ctrl_wb_dat_i,
    input   wire                    ctrl_wb_we_i,
    input   wire    [3:0]           ctrl_wb_sel_i,
    input   wire                    ctrl_wb_stb_i,
    output  reg                     ctrl_wb_ack_o,

    // PHY (ddr3_phy_slow controller side)
    input   wire                    phy_ce,
    output  wire                    phy_reset_n,
    output  wire                    phy_cke,
    output  wire                    phy_odt,
    output  wire    [2:0]           phy_cmd,
    output  wire    [ROW_W-1:0]     phy_addr,
    output  wire    [BA_W-1:0]      phy_ba,
    output  wire                    phy_wr,
    output  wire    [1:0]           phy_wr_ph,
    output  wire    [8*DQ_W-1:0]    phy_wdata,
    output  wire    [DQ_W-1:0]      phy_wmask,
    output  wire                    phy_rd,
    output  wire    [1:0]           phy_rd_ph,
    output  wire    [LANES*CAL_DW-1:0] phy_rd_delay,
    output  wire    [1:0]           phy_wr_shift,
    input   wire                    phy_rvalid,
    input   wire    [8*DQ_W-1:0]    phy_rdata,
    input   wire                    phy_ready
);
    localparam WORDS = 32 / DQ_W;               // controller words per Wishbone word

    initial begin
        if (DQ_W != 16 && DQ_W != 32) $error("DDR3_wb: DQ_W must be 16 or 32");
    end

    ////////////////////////////////////////////////////////////
    // Controller
    reg                     c_rd, c_wr, c_refresh;
    reg     [DRAM_AW-1:0]   c_addr;
    reg     [DQ_W-1:0]      c_din;
    reg     [LANES-1:0]     c_be;
    wire    [DQ_W-1:0]      c_dout;
    wire    [8*DQ_W-1:0]    c_dout_burst;
    wire                    c_data_ready, c_busy, c_init_done, c_calib_done, c_calib_fail;

    ddr3_ctrl #(
        .DQ_W(DQ_W), .ROW_W(ROW_W), .COL_W(COL_W), .BA_W(BA_W),
        .NPH(1), .TCK_PS(2_000_000_000 / (CLK_FREQ / 1000)), .CL(6), .CWL(6), .NWR(5), .DLL_OFF(1),
        .T_RFC_PS(T_RFC_PS), .CAL_DW(CAL_DW), .CAL_STEPS(2 ** CAL_DW), .CAL_PER_LANE(1), .SIM(SIM)
    ) ctrl (
        .clk(wb_clk_i), .ce(phy_ce), .rst(wb_rst_i),
        .rd(c_rd), .wr(c_wr), .refresh(c_refresh), .addr(c_addr), .din(c_din), .be(c_be),
        .dout(c_dout), .dout_burst(c_dout_burst), .data_ready(c_data_ready), .busy(c_busy),
        .init_done(c_init_done), .calib_done(c_calib_done), .calib_fail(c_calib_fail),
        .phy_reset_n(phy_reset_n), .phy_cke(phy_cke), .phy_odt(phy_odt),
        .phy_cmd(phy_cmd), .phy_addr(phy_addr), .phy_ba(phy_ba),
        .phy_wr(phy_wr), .phy_wr_ph(phy_wr_ph), .phy_wdata(phy_wdata), .phy_wmask(phy_wmask),
        .phy_rd(phy_rd), .phy_rd_ph(phy_rd_ph), .phy_rd_delay(phy_rd_delay), .phy_wr_shift(phy_wr_shift),
        .phy_rvalid(phy_rvalid), .phy_rdata(phy_rdata), .phy_ready(phy_ready)
    );

    `UNUSED_VAR(c_dout)
    `UNUSED_VAR(c_data_ready)
    `UNUSED_VAR(c_init_done)

    ////////////////////////////////////////////////////////////
    // Refresh: one due every 7.8 us, issued before the next access
    localparam REFI = CLK_FREQ / 1000 * 78 / 10000;
    reg     [15:0]  refi_cnt;
    reg             refresh_due;

    ////////////////////////////////////////////////////////////
    // Request sequencer. A request pulse goes out while the controller is idle; REQ
    // waits the cycle in which the controller takes it (busy rises a cycle later), WAIT
    // until it is done (busy low again).
    localparam S_IDLE = 2'd0, S_REQ = 2'd1, S_WAIT = 2'd2;
    reg     [1:0]           state;
    reg                     op_refresh;         // the operation in flight is a refresh
    reg     [WORDS-1:0]     todo;               // write halves still to go

    // controller word address of the Wishbone word's first half, and its beat in the burst
    wire    [DRAM_AW-1:0]   base = DRAM_AW'(mem_wb_adr_i) << $clog2(WORDS);
    wire    [2:0]           beat = base[2:0];

    // halves of the Wishbone word with at least one byte enabled
    wire    [WORDS-1:0]     sel_halves;
    genvar g;
    generate for (g = 0; g < WORDS; g = g + 1) begin : g_halves
        assign sel_halves[g] = |mem_wb_sel_i[LANES*g +: LANES];
    end endgenerate

    // next write: the lowest half still to go (from sel in IDLE, from todo in WAIT)
    wire    [WORDS-1:0]     halves = (state == S_IDLE) ? sel_halves : todo;
    wire                    wh     = (WORDS == 2) && !halves[0];
    wire    [WORDS-1:0]     w_todo = halves & ~(WORDS'(1) << wh);

    always @(posedge wb_clk_i) begin
        if (wb_rst_i) begin
            state           <= S_IDLE;
            mem_wb_ack_o    <= 1'b0;
            c_rd            <= 1'b0;
            c_wr            <= 1'b0;
            c_refresh       <= 1'b0;
            refi_cnt        <= 16'd0;
            refresh_due     <= 1'b0;
        end else begin
            c_rd            <= 1'b0;
            c_wr            <= 1'b0;
            c_refresh       <= 1'b0;
            mem_wb_ack_o    <= 1'b0;

            if (refi_cnt == 16'(REFI - 1)) begin
                refi_cnt    <= 16'd0;
                refresh_due <= 1'b1;
            end else
                refi_cnt    <= refi_cnt + 16'd1;

            case (state)
            S_IDLE: if (!c_busy) begin
                if (refresh_due) begin
                    c_refresh   <= 1'b1;
                    refresh_due <= 1'b0;
                    op_refresh  <= 1'b1;
                    state       <= S_REQ;
                end else if (mem_wb_stb_i && !mem_wb_ack_o) begin
                    op_refresh  <= 1'b0;
                    if (!mem_wb_we_i) begin
                        c_rd    <= 1'b1;
                        c_addr  <= base;
                        state   <= S_REQ;
                    end else if (|sel_halves) begin
                        c_wr    <= 1'b1;
                        c_addr  <= base + DRAM_AW'(wh);
                        c_din   <= mem_wb_dat_i[DQ_W*wh +: DQ_W];
                        c_be    <= mem_wb_sel_i[LANES*wh +: LANES];
                        todo    <= w_todo;
                        state   <= S_REQ;
                    end else
                        mem_wb_ack_o <= 1'b1;   // no byte enabled
                end
            end

            S_REQ: state <= S_WAIT;

            S_WAIT: if (!c_busy) begin
                if (op_refresh)
                    state <= S_IDLE;
                else if (!mem_wb_we_i) begin
                    mem_wb_dat_o <= c_dout_burst[DQ_W*beat +: 32];
                    mem_wb_ack_o <= 1'b1;
                    state        <= S_IDLE;
                end else if (|todo) begin
                    c_wr         <= 1'b1;
                    c_addr       <= base + DRAM_AW'(wh);
                    c_din        <= mem_wb_dat_i[DQ_W*wh +: DQ_W];
                    c_be         <= mem_wb_sel_i[LANES*wh +: LANES];
                    todo         <= w_todo;
                    state        <= S_REQ;
                end else begin
                    mem_wb_ack_o <= 1'b1;
                    state        <= S_IDLE;
                end
            end

            default: state <= S_IDLE;
            endcase
        end
    end

    ////////////////////////////////////////////////////////////
    // Registers
    always @(posedge wb_clk_i) begin
        if (wb_rst_i)
            ctrl_wb_ack_o <= 1'b0;
        else
            ctrl_wb_ack_o <= ctrl_wb_stb_i & !ctrl_wb_ack_o;

        case (ctrl_wb_adr_i)
            2'b00:   ctrl_wb_dat_o <= {30'd0, c_calib_fail, c_calib_done & !c_calib_fail};
            2'b01:   ctrl_wb_dat_o <= 32'(phy_rd_delay);
            default: ctrl_wb_dat_o <= 32'd0;
        endcase
    end

    `UNUSED_VAR(ctrl_wb_dat_i)
    `UNUSED_VAR(ctrl_wb_we_i)
    `UNUSED_VAR(ctrl_wb_sel_i)
endmodule
