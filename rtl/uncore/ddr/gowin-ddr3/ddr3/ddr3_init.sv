`ifndef module_ddr3_init
`define module_ddr3_init

// DDR3 power-up and initialization (JEDEC JESD79-3F 3.3.1).
//
// RESET# low for 200us, CKE low for another 500us, then after tXPR the mode
// registers in the order MR2, MR3, MR1, MR0, then ZQCL. The commands go
// through ddr3_cmd, which also waits tMOD and tZQinit (and with it tDLLK).
module ddr3_init #(
    parameter ROW_W   = 15,
    parameter BA_W    = 3,
    parameter CTL_PS  = 10000,          // controller cycle (ce period)
    parameter T_XPR_PS = 270000,        // tRFC + 10ns
    parameter CL      = 6,
    parameter CWL     = 5,
    parameter NWR     = 6,              // write recovery, 5..8, 10, 12, 14, 16
    parameter DLL_OFF = 0,              // DLL-off mode, requires CL=CWL=6 and no ODT
    parameter [1:0] RTT_WR = 2'b00,     // dynamic ODT during writes: 01 60ohm, 10 120ohm
    parameter SIM     = 0               // short reset waits for simulation
) (
    input                   clk,
    input                   ce,
    input                   rst,

    output reg              reset_n,
    output reg              cke,
    output reg              done,

    output reg              op_valid,
    input                   op_ready,
    output reg        [2:0] op,
    output reg   [BA_W-1:0] op_ba,
    output reg  [ROW_W-1:0] op_a,
    input                   op_done
);

initial begin
    reset_n = 0;
    cke = 0;
    done = 0;
    op_valid = 0;
    op = 0;
    op_ba = 0;
    op_a = 0;
end

localparam [2:0] OP_MRS = 3'd0, OP_ZQCL = 3'd1;

localparam integer C_RESET = SIM ? 20 : (200_000_000 + CTL_PS - 1) / CTL_PS;
localparam integer C_CKE   = SIM ? 20 : (500_000_000 + CTL_PS - 1) / CTL_PS;
localparam integer C_XPR   = (T_XPR_PS + CTL_PS - 1) / CTL_PS + 1;
localparam integer W = $clog2(C_CKE + 1);

// MR0: fixed BL8, sequential, CL, DLL reset (DLL on only), write recovery
localparam [2:0] WR_CODE = NWR <= 8 ? 3'(NWR - 4) : NWR == 16 ? 3'd0 : 3'(NWR / 2);
localparam [ROW_W-1:0] MR0 = ROW_W'({WR_CODE, DLL_OFF ? 1'b0 : 1'b1, 1'b0, 3'(CL - 4), 1'b0, 1'b0, 2'b00});
// MR1: DLL enable/disable, 40 ohm drive, RTT_NOM off, AL=0
localparam [ROW_W-1:0] MR1 = ROW_W'(DLL_OFF ? 1 : 0);
// MR2: CWL, dynamic ODT
localparam [ROW_W-1:0] MR2 = ROW_W'({RTT_WR, 3'b000, 3'(CWL - 5), 3'b000});
localparam [ROW_W-1:0] MR3 = '0;

initial begin
    if (DLL_OFF && (CL != 6 || CWL != 6)) $error("ddr3_init: DLL-off mode requires CL=CWL=6");
    if (DLL_OFF && RTT_WR != 0) $error("ddr3_init: no ODT in DLL-off mode");
end

localparam [2:0] S_RESET = 0, S_CKE = 1, S_XPR = 2, S_MRS = 3, S_WAIT = 4, S_DONE = 5;
reg [2:0] state;
reg [W-1:0] cnt;
reg [2:0] step;
initial begin
    state = S_RESET;
    cnt = 0;
    step = 0;
end

always @(posedge clk) begin
    if (rst) begin
        state <= S_RESET;
        cnt <= 0;
        step <= 0;
        reset_n <= 0;
        cke <= 0;
        done <= 0;
        op_valid <= 0;
    end else if (ce) begin
        cnt <= cnt + 1'd1;
        case (state)
        S_RESET: if (cnt == W'(C_RESET)) begin
            reset_n <= 1;
            cnt <= 0;
            state <= S_CKE;
        end
        S_CKE: if (cnt == W'(C_CKE)) begin
            cke <= 1;
            cnt <= 0;
            state <= S_XPR;
        end
        S_XPR: if (cnt == W'(C_XPR)) state <= S_MRS;
        S_MRS: if (op_ready) begin
            op_valid <= 1;
            case (step)
            3'd0: begin op <= OP_MRS; op_ba <= 2; op_a <= MR2; end
            3'd1: begin op <= OP_MRS; op_ba <= 3; op_a <= MR3; end
            3'd2: begin op <= OP_MRS; op_ba <= 1; op_a <= MR1; end
            3'd3: begin op <= OP_MRS; op_ba <= 0; op_a <= MR0; end
            default: begin op <= OP_ZQCL; op_ba <= 0; op_a <= ROW_W'(1 << 10); end
            endcase
            state <= S_WAIT;
        end
        S_WAIT: begin
            op_valid <= 0;
            if (op_done) begin
                step <= step + 1'd1;
                state <= step == 3'd4 ? S_DONE : S_MRS;
            end
        end
        default: done <= 1;
        endcase
    end
end

endmodule

`endif /* module_ddr3_init */
