`ifndef module_ddr3_phy_slow
`define module_ddr3_phy_slow

// Slow DDR3 PHY without PLL, IO serializers or delay lines.
//
// clk is the beat clock: CK runs at clk/2, so every clk cycle is one data beat
// (50 MHz board clock -> CK 25 MHz, 50 MT/s). That is far below the 300 MHz
// minimum of the DRAM's DLL, so the DRAM has to run in DLL-off mode (CL=CWL=6,
// no ODT), see ddr3_ctrl DLL_OFF.
//
// The controller runs on every other clk cycle (ce), one DRAM clock per cycle
// (NPH=1). Commands change on the ce edge and CK rises half a CK later, so
// the command bits are centered on the rising CK edge.
// Writes: DQS toggles on clk rising edges in line with CK, DQ changes on clk
// falling edges, centered between the DQS edges.
// Without ODT, an undriven DQS/DQS# pair floats to about the same level, and
// the DRAM can see spurious strobe edges that misplace the next write's beats.
// So DQS is driven low (DQS# high) all the time except during reads.
// CK and DQS are pseudo-differential pairs: if the two pins switch a few ns
// apart, the DRAM's differential receivers see no difference meanwhile and can
// produce extra edges. With ELVDS the pairs come from Gowin's emulated LVDS
// buffers, which drive both pins from one signal (IO_TYPE e.g. LVCMOS15D).
// Where those can't be used (apicula doesn't allow them on true LVDS pins),
// IOBFF drives every pin from a register in its IO block instead, so that the
// routing can't skew a pair either; otherwise the pins come from separate
// fabric registers.
// Reads: DQS is ignored. DQ is sampled on both clk edges and every byte lane
// picks the beats at its own offset from the nominal read latency, in steps of
// half a clk cycle (rd_delay, set by read calibration).
module ddr3_phy_slow #(
    parameter DQ_W  = 32,
    parameter ROW_W = 15,
    parameter BA_W  = 3,
    parameter CL    = 6,
    parameter CWL   = 6,
    parameter DW    = 3,                // read delay bits per lane
    parameter ELVDS = 0,                // Gowin ELVDS_OBUF/ELVDS_IOBUF for CK and DQS
    parameter IOBFF = 0,                // without ELVDS: CK, DQS from Gowin OBUF/IOBUF with IO block registers
    localparam LANES = DQ_W / 8,
    localparam STEPS = 2 ** DW
) (
    input                       clk,
    output reg                  ce,

    // controller side
    input                       phy_reset_n,
    input                       phy_cke,
    input                       phy_odt,
    input                 [2:0] phy_cmd,
    input           [ROW_W-1:0] phy_addr,
    input            [BA_W-1:0] phy_ba,
    input                       phy_wr,
    input                 [1:0] phy_wr_ph,
    input          [8*DQ_W-1:0] phy_wdata,
    input            [DQ_W-1:0] phy_wmask,
    input                       phy_rd,
    input                 [1:0] phy_rd_ph,
    input      [LANES*DW-1:0]   phy_rd_delay,
    input                 [1:0] phy_wr_shift,   // unused, writes are always aligned
    output reg                  phy_rvalid,
    output reg   [8*DQ_W-1:0]   phy_rdata,
    output                      phy_ready,
    output          [LANES-1:0] dqs_in,     // DQS as received, for diagnostics

    // DRAM side
    output                      ddr_ck,
    output                      ddr_ck_n,
    output                      ddr_reset_n,
    output                      ddr_cke,
    output                      ddr_cs_n,
    output                      ddr_ras_n,
    output                      ddr_cas_n,
    output                      ddr_we_n,
    output                      ddr_odt,
    output          [BA_W-1:0]  ddr_ba,
    output         [ROW_W-1:0]  ddr_addr,
    output         [LANES-1:0]  ddr_dm,
    inout          [LANES-1:0]  ddr_dqs,
    inout          [LANES-1:0]  ddr_dqs_n,
    inout           [DQ_W-1:0]  ddr_dq
);

initial begin
    ce = 0;
    phy_rvalid = 0;
    phy_rdata = '0;
end

// ce marks the clk edge that starts a DRAM clock, CK rises on the next edge
reg ck_q, ck_n_q;
initial begin
    ck_q = 0;
    ck_n_q = 1;
end
always @(posedge clk) begin
    ce <= ~ce;
    ck_q <= ~ce;
    ck_n_q <= ce;
end
if (ELVDS) begin : g_ck_elvds
    ELVDS_OBUF buf_ck(.I(ck_q), .O(ddr_ck), .OB(ddr_ck_n));
end else if (IOBFF) begin : g_ck_iob
    // keep: a register per pin; init 1: not merged with ce, which has the same input
    reg ck_io, ck_n_io;
    initial begin
        ck_io = 1;
        ck_n_io = 1;
    end
    (* keep *) always @(posedge clk) begin
        ck_io <= ~ce;
        ck_n_io <= ce;
    end
    (* IOBFF *) OBUF buf_ck(.I(ck_io), .O(ddr_ck));
    (* IOBFF *) OBUF buf_ck_n(.I(ck_n_io), .O(ddr_ck_n));
end else begin : g_ck
    assign ddr_ck = ck_q;
    assign ddr_ck_n = ck_n_q;
end

assign ddr_reset_n = phy_reset_n;
assign ddr_cke = phy_cke;
assign ddr_cs_n = 1'b0;
assign {ddr_ras_n, ddr_cas_n, ddr_we_n} = phy_cmd;
assign ddr_odt = phy_odt;
assign ddr_ba = phy_ba;
assign ddr_addr = phy_addr;
assign phy_ready = 1'b1;

// Write burst, counted in clk edges s after the ce edge that ends the WRITE
// command's DRAM clock (s=0). CK rises at s=2n-1 for the n-th DRAM clock after
// the command, the first DQS rising edge is due at the CWL-th one.
localparam integer S_DQS_1ST = 2 * CWL - 1;
localparam integer S_DQS_OFF = 2 * CWL + 8;          // after the postamble
localparam integer S_DQ_1ST  = 2 * CWL - 2;          // beat 0, output on the falling edge
localparam SWW = $clog2(S_DQS_OFF + 2);
localparam bit DQS_ODD = S_DQS_1ST % 2 == 1;   // DQS is high on the edges with this parity

reg            w_act;
reg  [SWW-1:0] ws;
reg [8*DQ_W-1:0] w_data;
reg   [DQ_W-1:0] w_mask;
reg            dqs_q;
reg            dqs_oe;          // released while the DRAM may drive DQS, see the read capture
initial begin
    w_act = 0;
    ws = 0;
    w_data = '0;
    w_mask = '0;
    dqs_q = 0;
    dqs_oe = 1;
end
wire           dqs_oe_d;
// DQS: low but for four high pulses
wire           dqs_q_d = w_act && ws >= SWW'(S_DQS_1ST) && ws < SWW'(S_DQS_1ST + 8) && ws[0] == DQS_ODD;
reg            dq_oe_next;
reg [DQ_W-1:0] dq_next;
reg [LANES-1:0] dm_next;
initial begin
    dq_oe_next = 0;
    dq_next = '0;
    dm_next = '0;
end

always @(posedge clk) begin
    if (ce && phy_wr) begin
        w_act <= 1;
        ws <= 1;
        w_data <= phy_wdata;
        w_mask <= phy_wmask;
    end else if (w_act) begin
        ws <= ws + 1'd1;
        if (ws == SWW'(S_DQS_OFF)) w_act <= 0;
    end
    dqs_q <= dqs_q_d;
    if (w_act) begin
        // DQ for the next falling edge
        dq_oe_next <= ws >= SWW'(S_DQ_1ST) && ws < SWW'(S_DQ_1ST + 8);
        for (int j = 0; j < 8; j++)
            if (ws == SWW'(S_DQ_1ST + j)) begin
                dq_next <= w_data[DQ_W*j +: DQ_W];
                dm_next <= w_mask[LANES*j +: LANES];
            end
    end else
        dq_oe_next <= 0;
end

reg            dq_oe;
reg [DQ_W-1:0] dq_q;
reg [LANES-1:0] dm_q;
initial begin
    dq_oe = 0;
    dq_q = '0;
    dm_q = '0;
end
always @(negedge clk) begin
    dq_oe <= dq_oe_next;
    dq_q <= dq_next;
    dm_q <= dm_next;
end

for (genvar l = 0; l < LANES; l++) begin : g_dqs
    if (ELVDS) begin : g_elvds
        ELVDS_IOBUF buf_dqs(.I(dqs_q), .OEN(!dqs_oe), .O(dqs_in[l]), .IO(ddr_dqs[l]), .IOB(ddr_dqs_n[l]));
    end else if (IOBFF) begin : g_iob
        // keep: a register per pin and tristate, like dqs_q and !dqs_oe;
        // nextpnr only packs the output registers of an IOBUF whose input
        // drives just a register, and all registers of an IO block have to be
        // of the same type (init 0)
        reg p_io, n_io, oen_p, oen_n;
        initial begin
            p_io = 0;
            n_io = 0;
            oen_p = 0;
            oen_n = 0;
        end
        (* keep *) reg in_p, in_n;   // no other users
        initial begin
            in_p = 0;
            in_n = 0;
        end
        wire o_p, o_n;
        (* keep *) always @(posedge clk) begin
            p_io <= dqs_q_d;
            n_io <= !dqs_q_d;
            oen_p <= !dqs_oe_d;
            oen_n <= !dqs_oe_d;
            in_p <= o_p;
            in_n <= o_n;
        end
        assign dqs_in[l] = in_p;
        (* IOBFF *) IOBUF buf_dqs(.I(p_io), .OEN(oen_p), .O(o_p), .IO(ddr_dqs[l]));
        (* IOBFF *) IOBUF buf_dqs_n(.I(n_io), .OEN(oen_n), .O(o_n), .IO(ddr_dqs_n[l]));
    end else begin : g_se
        assign ddr_dqs[l]   = dqs_oe ? dqs_q : 1'bz;
        assign dqs_in[l]    = ddr_dqs[l];
        assign ddr_dqs_n[l] = dqs_oe ? ~dqs_q : 1'bz;
    end
end
assign ddr_dq    = dq_oe ? dq_q : 'z;
assign ddr_dm    = dm_q;

// Read capture. Samples are numbered in half clk cycles h after the ce edge
// that ends the READ command's DRAM clock; the n-th CK rising edge after the
// command is at h = 4*n-2. In DLL-off mode the DRAM starts beat 0 tDQSCK after
// the (CL-1)-th edge, not the CL-th (JESD79-3F 5.4), and tDQSCK (1-10ns) may be
// longer than tCK. Beat j of a lane is sampled at h = H0 + delay + 2*j, so the
// delays cover CL-1 up to CL plus a bit.
reg [DQ_W-1:0] dq_p, dq_n;
initial begin
    dq_p = '0;
    dq_n = '0;
end
always @(posedge clk) dq_p <= ddr_dq;
always @(negedge clk) dq_n <= ddr_dq;

localparam integer H0 = 4 * (CL - 1) - 2;
localparam integer U_END = (H0 + STEPS - 1 + 14) / 2 + 2;    // all samples are in
localparam UW = $clog2(U_END + 2);

reg           r_act;
reg [UW-1:0]  ru;               // clk edges since the start; samples 2u-2 and 2u-1 are ready
reg [8*DQ_W-1:0] r_data;
initial begin
    r_act = 0;
    ru = 0;
    r_data = '0;
end

assign dqs_oe_d = ce && phy_rd ? 1'b0 : r_act && ru == UW'(U_END) ? 1'b1 : dqs_oe;
always @(posedge clk) dqs_oe <= dqs_oe_d;

always @(posedge clk) begin
    if (ce && phy_rd) begin
        r_act <= 1;
        ru <= 1;
    end else if (r_act) begin
        ru <= ru + 1'd1;
        for (int l = 0; l < LANES; l++)
            for (int j = 0; j < 8; j++) begin
                if (32'(ru) * 2 - 2 == 32'(H0) + 32'(phy_rd_delay[DW*l +: DW]) + 2 * j)
                    r_data[DQ_W*j + 8*l +: 8] <= dq_p[8*l +: 8];
                if (32'(ru) * 2 - 1 == 32'(H0) + 32'(phy_rd_delay[DW*l +: DW]) + 2 * j)
                    r_data[DQ_W*j + 8*l +: 8] <= dq_n[8*l +: 8];
            end
        if (ru == UW'(U_END)) begin
            r_act <= 0;
            phy_rvalid <= 1;
            phy_rdata <= r_data;
        end
    end
    // the controller takes the data on its next ce edge
    if (phy_rvalid && ce) phy_rvalid <= 0;
end

endmodule

`endif /* module_ddr3_phy_slow */
