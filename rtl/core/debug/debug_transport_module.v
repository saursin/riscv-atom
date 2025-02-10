`default_nettype none

module debug_transport_module #(
    parameter TAP_IDCODE            = 32'h0000_0001,    // JTAG IDCODE
    parameter DMI_ADDRW             = 8,
    parameter DMI_DATAW             = 32,
    parameter DTMCS_IDLE_HINT       = 3'd0,     // Number of cycles debugger should spend in idle state to avoid dmistat == 3
    parameter DEBUG_SPEC_VERSION    = 4'b0001   // 0: version 0.11, 1: version 0.13, 15: none  
) (
    // JTAG interface
    input  wire      tck_i,
    input  wire      trst_n_i,
    input  wire      tms_i,
    input  wire      tdi_i,
    output reg       tdo_o,

    // Wishbone interface
    output  wire [DMI_ADDRW-1:0]    dmi_wb_adr_o,
    output  wire [DMI_DATAW-1:0]    dmi_wb_dat_o,
    /* verilator lint_off UNUSED */
    input   wire [DMI_DATAW-1:0]    dmi_wb_dat_i,
    /* verilator lint_off UNUSED */
    output  wire                    dmi_wb_cyc_o,
    output  wire                    dmi_wb_stb_o,
    output  wire                    dmi_wb_we_o,
    output  wire [DMI_DATAW/8-1:0]  dmi_wb_sel_o,
    input   wire                    dmi_wb_err_i,
    /* verilator lint_off UNUSED */
    input   wire                    dmi_wb_ack_i
    /* verilator lint_off UNUSED */
);
   
    localparam NUM_CUST_REGS    = 2;            // DTMCS, DMI
    localparam ABITS            = DMI_ADDRW-2;  // -2 cuz wb is byte addressable while dm registers are word addressed

    localparam REG_DTMCS_ADDR   = 5'h10;
    localparam REG_DTMCS_WIDTH  = 8'd32;
    
    localparam REG_DMI_ADDR     = 5'h11;
    localparam REG_DMI_WIDTH    = ABITS + DMI_DATAW + 2;
    localparam REG_DMI_WIDTH1   = REG_DMI_WIDTH[7:0];

    localparam IR_WIDTH         = 5;
    localparam DR_WIDTH         = REG_DMI_WIDTH;

    ////////////////////////////////////////
    // JTAG Tap controller
    wire [$clog2(NUM_CUST_REGS)-1:0] cust_rg_addr_o;
    wire                             cust_rg_val_o;
    wire [DR_WIDTH-1:0]              cust_rg_dat_o;
    wire [DR_WIDTH-1:0]              cust_rg_dat_i;
    wire                             cust_rg_dat_re_o;
    wire                             cust_rg_dat_we_o;

    jtag_tap #(
        .IDCODE(TAP_IDCODE),
        .IR_WIDTH(IR_WIDTH),
        .DR_WIDTH(DR_WIDTH),
        .NUM_CUST_REGS(NUM_CUST_REGS),
        .CUST_REG_ADDRS({REG_DMI_ADDR, REG_DTMCS_ADDR}),
        .CUST_REG_WIDTHS({REG_DMI_WIDTH1, REG_DTMCS_WIDTH})
    ) tap (
        .tck_i              (tck_i),
        .trst_n_i           (trst_n_i),
        .tms_i              (tms_i),
        .tdi_i              (tdi_i),
        .tdo_o              (tdo_o),
        .cust_rg_addr_o     (cust_rg_addr_o),
        .cust_rg_val_o      (cust_rg_val_o),
        .cust_rg_dat_o      (cust_rg_dat_o),
        .cust_rg_dat_i      (cust_rg_dat_i),
        .cust_rg_dat_re_o   (cust_rg_dat_re_o),
        .cust_rg_dat_we_o   (cust_rg_dat_we_o)
    );


    wire sel_rg_dtmcs       = (cust_rg_addr_o == 1'b0);
    wire sel_rg_dmi         = (cust_rg_addr_o == 1'b1);

    ////////////////////////////////////////
    // DTMCS register
    // no need to store any state here, all bits are combinational

    localparam DMISTAT_NOERROR  = 2'd0; // No error
    localparam DMISTAT_OPFAIL   = 2'd2; // Operation failed
    localparam DMISTAT_OPBUSYAT = 2'd2; // Operation attempted while busy

    wire dtmcs_written = cust_rg_val_o && cust_rg_dat_we_o && sel_rg_dtmcs;

    wire dtmcs_dmihardreset         = dtmcs_written && cust_rg_dat_o[17];
    wire dtmcs_dmireset             = dtmcs_written && cust_rg_dat_o[16];
    wire [2:0]  dtmcs_idle          = DTMCS_IDLE_HINT;
    wire [1:0]  dtmcs_dmistat       = wb_status;
    wire [5:0]  dtmcs_abits         = ABITS;
    wire [3:0]  dtmcs_version       = DEBUG_SPEC_VERSION;

    wire [31:0] dtmcs_readval = {17'd0, dtmcs_idle, dtmcs_dmistat, dtmcs_abits, dtmcs_version};

    
    ////////////////////////////////////////
    // DMI register
    reg  [31:0]             dmi_data;
    reg  [ABITS-1:0]        dmi_address;

    wire [(ABITS+32+2)-1:0] dmi_readval = {dmi_address, dmi_data, wb_status};   // This reg determines tap DR width

    wire dmi_written = cust_rg_val_o && cust_rg_dat_we_o && sel_rg_dmi;

    always @(posedge tck_i) begin
        if(!trst_n_i) begin
            dmi_data    <= 32'd0;
            dmi_address <= 'd0;
        end
        else begin
            if(dmi_written) begin
                dmi_data    <= cust_rg_dat_o[33:2];
                dmi_address <= cust_rg_dat_o[34+:ABITS];
            end
        end
    end
    
    wire [1:0] dmi_op      = cust_rg_dat_o[1:0];

    // Select DMI or DTMCS for reading
    assign cust_rg_dat_i    = sel_rg_dtmcs ? {{DR_WIDTH-32{1'b0}}, dtmcs_readval} : 
                              sel_rg_dmi   ? dmi_readval   : 0;

    ////////////////////////////////////////
    // Wishbone master
    assign dmi_wb_adr_o = {dmi_address, 2'b00};
    assign dmi_wb_dat_o = dmi_data;
    assign dmi_wb_sel_o = 4'b1111;

    localparam WB_IDLE  =  1'b0;
    localparam WB_ACTIV =  1'b1;
   
    localparam WBSTAT_OK            =  2'b00;
    localparam WBSTAT_ERR           =  2'b10;
    localparam WBSTAT_BUSYATTEMPT   =  2'b11;

    reg         wb_state;
    reg [1:0]   wb_status;
    always @(posedge tck_i) begin
        if(!trst_n_i || dtmcs_dmihardreset) begin   // hard reset
            dmi_wb_cyc_o <= 0;
            dmi_wb_stb_o <= 0;
            dmi_wb_we_o <= 0;
            wb_state <= WB_IDLE;
            wb_status <= WBSTAT_OK;
        end
        else begin
            if(dtmcs_dmireset)
                wb_status <= WBSTAT_OK;             // Clear sticky bit

            case (wb_state)
                WB_IDLE: begin
                    if(dmi_written) begin 
                        if(dmi_op == 2'b00) begin
                            // NOP
                        end
                        else if (dmi_op == 2'b01) begin
                            // Start a read access
                            dmi_wb_cyc_o <= 1;
                            dmi_wb_stb_o <= 1;
                            dmi_wb_we_o <= 0;
                            wb_state <= WB_ACTIV;
                        end
                        else if (dmi_op == 2'b10) begin
                            // Start a write access
                            dmi_wb_cyc_o <= 1;
                            dmi_wb_stb_o <= 1;
                            dmi_wb_we_o <= 1;
                            wb_state <= WB_ACTIV;
                        end
                    end
                end
                WB_ACTIV: begin
                    if(dmi_written && dmi_op != 2'b00) begin
                        // Request while transaction ongoing.
                        // Set sticky bit, request ignored
                        wb_status <= WBSTAT_BUSYATTEMPT;
                    end

                    if(dmi_wb_ack_i) begin
                        // We got ack, transaction is complete
                        dmi_wb_cyc_o <= 0;
                        dmi_wb_stb_o <= 0;
                        wb_state <= WB_IDLE;
                        wb_status <= WBSTAT_OK;
                        if(!dmi_wb_we_o) begin
                            dmi_data <= dmi_wb_dat_i;
                        end
                    end
                    else if (dmi_wb_err_i) begin
                        // Error during transaction, set status as err
                        dmi_wb_cyc_o <= 0;
                        dmi_wb_stb_o <= 0;
                        wb_state <= WB_IDLE;
                        wb_status <= WBSTAT_ERR;
                    end
                end
            endcase
        end
    end

endmodule
