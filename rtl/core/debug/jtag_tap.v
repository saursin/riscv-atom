// Paramatrized JTAG Tap controller

`default_nettype none

module jtag_tap #(
    parameter IDCODE            = 32'h0000_0000, // JTAG IDCODE
    parameter IR_WIDTH          = 5,             // Instruction register width
    parameter DR_WIDTH          = 32,            // Data register width
    parameter NUM_CUST_REGS     = 0,             // Number of custom registers
    parameter CUST_REG_ADDRS    = 0,             // Custom register addreeses :{... IR_WIDTH'h<ADDR1>, IR_WIDTH'h<ADDR1>}
    parameter CUST_REG_WIDTHS   = 0,             // Custom register widths   : {... clog2(DR_WIDTH)'h<WIDTH1>, clog2(DR_WIDTH)'h<WIDTH0>}

    parameter CUST_REGIF_ADDRW  = (NUM_CUST_REGS > 0 ? $clog2(NUM_CUST_REGS) : 1)
) (
    // JTAG interface
    input  wire      tck_i,
    input  wire      trst_n_i,
    input  wire      tms_i,
    input  wire      tdi_i,
    output reg       tdo_o,

    // Custom register interface
    output  reg  [CUST_REGIF_ADDRW-1:0]      cust_rg_addr_o,
    output  reg                              cust_rg_val_o,
    output  wire [DR_WIDTH-1:0]              cust_rg_dat_o,
    input   wire [DR_WIDTH-1:0]              cust_rg_dat_i,
    output  wire                             cust_rg_dat_re_o,
    output  wire                             cust_rg_dat_we_o
);
    localparam ADDR_IDCODE          = 5'h1;     // 32 bit register

    ////////////////////////////////////////////////////////////////////////////
    // Tap controller state machine
    localparam TS_TEST_LOGIC_RST    = 4'd0;
    localparam TS_RUN_TEST_IDLE     = 4'd1;
    localparam TS_SELECT_DR_SCAN    = 4'd2;
    localparam TS_CAPTURE_DR        = 4'd3;
    localparam TS_SHIFT_DR          = 4'd4;
    localparam TS_EXIT1_DR          = 4'd5;
    localparam TS_PAUSE_DR          = 4'd6;
    localparam TS_EXIT2_DR          = 4'd7;
    localparam TS_UPDATE_DR         = 4'd8;
    localparam TS_SELECT_IR_SCAN    = 4'd9;
    localparam TS_CAPTURE_IR        = 4'd10;
    localparam TS_SHIFT_IR          = 4'd11;
    localparam TS_EXIT1_IR          = 4'd12;
    localparam TS_PAUSE_IR          = 4'd13;
    localparam TS_EXIT2_IR          = 4'd14;
    localparam TS_UPDATE_IR         = 4'd15;

    reg [3:0] tap_state;

    always @(posedge tck_i) begin
        if (!trst_n_i) begin
            tap_state <= TS_TEST_LOGIC_RST;
        end 
        else begin
            case (tap_state)
                TS_TEST_LOGIC_RST:  tap_state <= (tms_i ? TS_TEST_LOGIC_RST : TS_RUN_TEST_IDLE);
                TS_RUN_TEST_IDLE:   tap_state <= (tms_i ? TS_SELECT_DR_SCAN : TS_RUN_TEST_IDLE);

                TS_SELECT_DR_SCAN:  tap_state <= (tms_i ? TS_SELECT_IR_SCAN : TS_CAPTURE_DR);
                TS_CAPTURE_DR:      tap_state <= (tms_i ? TS_EXIT1_DR       : TS_SHIFT_DR);
                TS_SHIFT_DR:        tap_state <= (tms_i ? TS_EXIT1_DR       : TS_SHIFT_DR);
                TS_EXIT1_DR:        tap_state <= (tms_i ? TS_UPDATE_DR      : TS_PAUSE_DR);
                TS_PAUSE_DR:        tap_state <= (tms_i ? TS_EXIT2_DR       : TS_PAUSE_DR);
                TS_EXIT2_DR:        tap_state <= (tms_i ? TS_UPDATE_DR      : TS_SHIFT_DR);
                TS_UPDATE_DR:       tap_state <= (tms_i ? TS_SELECT_DR_SCAN : TS_RUN_TEST_IDLE);

                TS_SELECT_IR_SCAN:  tap_state <= (tms_i ? TS_TEST_LOGIC_RST : TS_CAPTURE_IR);
                TS_CAPTURE_IR:      tap_state <= (tms_i ? TS_EXIT1_IR       : TS_SHIFT_IR);
                TS_SHIFT_IR:        tap_state <= (tms_i ? TS_EXIT1_IR       : TS_SHIFT_IR);
                TS_EXIT1_IR:        tap_state <= (tms_i ? TS_UPDATE_IR      : TS_PAUSE_IR);
                TS_PAUSE_IR:        tap_state <= (tms_i ? TS_EXIT2_IR       : TS_PAUSE_IR);
                TS_EXIT2_IR:        tap_state <= (tms_i ? TS_UPDATE_IR      : TS_SHIFT_IR);
                TS_UPDATE_IR:       tap_state <= (tms_i ? TS_SELECT_DR_SCAN : TS_RUN_TEST_IDLE);
                default:            tap_state <= TS_TEST_LOGIC_RST;
            endcase
        end
    end


    ////////////////////////////////////////////////////////////////////////////
    // Instruction Register
    reg [IR_WIDTH-1:0] reg_ir;
    reg [IR_WIDTH-1:0] reg_shift_ir;

    always @(posedge tck_i) begin
        if(!trst_n_i) begin
            reg_shift_ir    <= 'd0;
            reg_ir          <= ADDR_IDCODE;
        end
        else begin
            case(tap_state)
                TS_TEST_LOGIC_RST: begin
                    reg_shift_ir    <= 'd0;
                    reg_ir          <= ADDR_IDCODE;
                end
                TS_CAPTURE_IR:  begin 
                    reg_shift_ir    <= reg_ir;                                  // Capture IR into IR shift register
                end
                TS_SHIFT_IR:    begin
                    reg_shift_ir    <= {tdi_i, reg_shift_ir[IR_WIDTH-1:1]};     // Shift IR
                end
                TS_UPDATE_IR:   begin
                    reg_ir          <= reg_shift_ir;                            // Update IR
                end
                default: begin
                    reg_shift_ir    <= reg_shift_ir;
                    reg_ir          <= reg_ir;
                end
            endcase 
        end
    end

    // Check if custom register is selected & get its address
    always @(*) /* COMBINATORIAL */ begin
        cust_rg_val_o = 1'b0;
        cust_rg_addr_o = 1'b0;
        for (integer i = 0; i < NUM_CUST_REGS; i++) begin
            if(reg_ir == CUST_REG_ADDRS[(i*IR_WIDTH)+:IR_WIDTH]) begin
                cust_rg_val_o = 1;
                cust_rg_addr_o = i[CUST_REGIF_ADDRW-1:0];
            end
        end
    end

    ////////////////////////////////////////////////////////////////////////////
    // Data Register
    reg [DR_WIDTH-1:0] reg_shift_dr;

    localparam DR_WIDTH_BITS = $clog2(DR_WIDTH);

    always @(posedge tck_i) begin
        if(!trst_n_i) begin
            reg_shift_dr    <= 'd0;     // TODO: check if we also need to reset reg_dr
        end
        else begin
            case(tap_state)
                TS_CAPTURE_DR: begin
                    if(reg_ir == ADDR_IDCODE) begin
            			reg_shift_dr <= {{DR_WIDTH-32{1'b0}}, IDCODE};
                    end
                    else if (cust_rg_val_o)             // Load Custom register
                        reg_shift_dr <= cust_rg_dat_i;
                    else begin                          // Bypass
            			reg_shift_dr <= {DR_WIDTH{1'b0}};
                    end
                end

                TS_SHIFT_DR: begin
                    reg_shift_dr <= {tdi_i, reg_shift_dr[DR_WIDTH-1:1]};

                    // Shorten DR chain according to IR
                    if(reg_ir == ADDR_IDCODE) begin          // 32 bit idcode
                        reg_shift_dr[31] <= tdi_i;
                    end
                    else if (cust_rg_val_o) begin
                        reg_shift_dr[CUST_REG_WIDTHS[(DR_WIDTH_BITS*cust_rg_addr_o)+:DR_WIDTH_BITS]-1] <= tdi_i;
                    end
                    else begin                            // Bypass
                        reg_shift_dr[0] <= tdi_i;
                    end
                end

                default:
                    reg_shift_dr <= reg_shift_dr;
            endcase
        end
    end


    // Handle TDO
    always @ (negedge tck_i) begin
        if (!trst_n_i) begin
            tdo_o <= 1'b0;
        end 
        else begin
            case(tap_state)
                TS_SHIFT_IR: tdo_o <= reg_shift_ir[0];
                TS_SHIFT_DR: tdo_o <= reg_shift_dr[0];
                default:     tdo_o <= 1'b0;
            endcase
        end
    end

    assign cust_rg_dat_we_o = cust_rg_val_o && (tap_state == TS_UPDATE_DR);
    assign cust_rg_dat_re_o = cust_rg_val_o && (tap_state == TS_CAPTURE_DR);
    assign cust_rg_dat_o    = reg_shift_dr;

endmodule
