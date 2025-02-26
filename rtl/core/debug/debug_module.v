`default_nettype none

module debug_module #(
    parameter DMI_ADDRW     = 32,                   // Address width of the DMI
    parameter DMI_DATAW     = 32,                   // Data width of the DMI
    parameter N_HARTS       = 1,                    // Number of harts
    parameter NEXT_DM_ADDR  = 32'h0000_0000,        // Address of the next DM
    parameter PROGBUF_SIZE  = 2,                    // Number of words in the program buffer (2 words+impebreak sufficient for RV32I)
    parameter EN_IMPEBREAK  = 1                     // Enable implicit ebreak
)(
    input  wire     clk_i,
    input  wire     rst_i,

    output wire                     ndmreset_o,     // Non debug module reset
    input  wire                     ndmreset_ack_i, // Non debug module reset acknowledge
    output wire [N_HARTS-1:0]       debug_req_o,    // async debug request

    // Debug Transport Module Ifc
    /* verilator lint_off UNUSED */
    input  wire [DMI_ADDRW-1:0]     dm_wb_adr_i,
    input  wire [DMI_DATAW-1:0]     dm_wb_dat_i,
    output reg  [DMI_DATAW-1:0]     dm_wb_dat_o,
    input  wire                     dm_wb_cyc_i,
    input  wire                     dm_wb_stb_i,
    input  wire                     dm_wb_we_i,
    input  wire [DMI_DATAW/8-1:0]   dm_wb_sel_i,
    output wire                     dm_wb_err_o,
    output reg                      dm_wb_ack_o
    /* verilator lint_on UNUSED */
);

////////////////////////////////////////////////////////////////////////////////
// localparam ADDR_DATA0        = 6'h04;   // We only support 1 data register
localparam ADDR_DMCONTROL    = 6'h10;
localparam ADDR_DMSTATUS     = 6'h11;
localparam ADDR_HARTINFO     = 6'h12;
// localparam ADDR_HALTSUM1     = 6'h13;
// localparam ADDR_HALTSUM0     = 6'h40;   // We only support 1 halt summary register (32 harts)
localparam ADDR_HAWINDOWSEL  = 6'h14;
localparam ADDR_HAWINDOW     = 6'h15;
// localparam ADDR_ABSTRACTCS   = 6'h16;
// localparam ADDR_COMMAND      = 6'h17;
// localparam ADDR_ABSTRACTAUTO = 6'h18;
// localparam ADDR_CONFSTRPTR0  = 6'h19;
// localparam ADDR_CONFSTRPTR1  = 6'h1a;
// localparam ADDR_CONFSTRPTR2  = 6'h1b;
// localparam ADDR_CONFSTRPTR3  = 6'h1c;
// localparam ADDR_NEXTDM       = 6'h1d;
// localparam ADDR_PROGBUF0     = 6'h20;
// localparam ADDR_PROGBUF1     = 6'h21;
// localparam ADDR_SBCS         = 6'h38;
// localparam ADDR_SBADDRESS0   = 6'h39;
// localparam ADDR_SBDATA0      = 6'h3c;

`UNUSED_PARAM(NEXT_DM_ADDR)
`UNUSED_PARAM(PROGBUF_SIZE)
assign debug_req_o = 1'b0;


////////////////////////////////////////
// Wishbone logic
assign dm_wb_err_o = 0;

// Set Ack_o
always @(posedge clk_i) begin
  if (rst_i)
    dm_wb_ack_o <= 1'b0;
  else
    dm_wb_ack_o <= dm_wb_stb_i & !dm_wb_ack_o;
end

// Wb is byte addressed whereas DM registers are word addressed
// So we need to discard the lower 2 bits of the address
wire [5:0] wb_reg_addr = dm_wb_adr_i[7:2];

wire        wb_we       = dm_wb_cyc_i && dm_wb_stb_i && dm_wb_we_i;
/* verilator lint_off UNUSEDSIGNAL */
wire        wb_re       = dm_wb_cyc_i && dm_wb_stb_i && !dm_wb_we_i;
wire [3:0]  wb_wmask    = {4{wb_we}} & dm_wb_sel_i;
/* verilator lint_on UNUSEDSIGNAL */
// wire        we_dmstatus     = wb_we && (wb_reg_addr == ADDR_DMSTATUS);
wire        we_dmcontrol    = wb_we && (wb_reg_addr == ADDR_DMCONTROL);
wire        we_hawindowsel  = wb_we && (wb_reg_addr == ADDR_HAWINDOWSEL);
wire        we_hawindow     = wb_we && (wb_reg_addr == ADDR_HAWINDOW);

`UNUSED_VAR(we_hawindowsel)

function get_any;
    input [N_HARTS-1:0] in;
    begin
        get_any = in[hartsel] || (dmcontrol_hasel && |(in & hart_array_mask));
    end
endfunction

function get_all;
    input [N_HARTS-1:0] in;
    begin
        get_all = in[hartsel] && (!dmcontrol_hasel && ~|(in & hart_array_mask));
    end
endfunction


////////////////////////////////////////
reg         dmcontrol_dmactive;


////////////////////////////////////////
// Hart Selection logic

// Number of bits required to represent harts
// NOTE: We only support N_HARTS <= 1024 (HARTSELLEN <= 10)
localparam HARTSELLEN = N_HARTS > 1 ? $clog2(N_HARTS) : 1;    

// Hartsel selects the hart to be operated on. Its used to select single hart.
reg  [HARTSELLEN-1:0]  hartsel;                         
always @(posedge clk_i) begin
    if (rst_i || !dmcontrol_dmactive)
        hartsel <= 'd0;
    else begin
        if (dmcontrol_dmactive && we_dmcontrol) begin
            hartsel <= dm_wb_dat_i[16+:HARTSELLEN];
        end
    end
end

// Hart array mask is used to select multiple harts
reg  [N_HARTS-1:0]      hart_array_mask;
always @(posedge clk_i) begin
    if (rst_i || !dmcontrol_dmactive)
        hart_array_mask <= 'd0;
    else begin
        if (dmcontrol_dmactive && we_hawindow) begin
            hart_array_mask <= dm_wb_dat_i[N_HARTS-1:0];
        end
    end
end

wire [N_HARTS-1:0] dmstatus_havereset   = 'd0; // FIXME: 
wire [N_HARTS-1:0] dmstatus_resumeack   = 'd0; // FIXME: 
wire [N_HARTS-1:0] hart_available       = {N_HARTS{ndmreset_ack_i}};  // All harts are available  // FIXME:
wire [N_HARTS-1:0] hart_running         = 'd0;    // FIXME:
wire [N_HARTS-1:0] hart_halted          = 'd0;     // FIXME:

////////////////////////////////////////////////////////////////////////////////
// Debug Module Registers

////////////////////////////////////////
// 0x11: Dmstatus register
wire        dmstatus_impebreak          = EN_IMPEBREAK;     // Implicit ebreak
wire        dmstatus_allhavereset       = get_all(dmstatus_havereset);
wire        dmstatus_anyhavereset       = get_any(dmstatus_havereset);
wire        dmstatus_allresumeack       = get_all(dmstatus_resumeack);
wire        dmstatus_anyresumeack       = get_any(dmstatus_resumeack);
wire        dmstatus_allnonexistent     = hartsel >= N_HARTS && !(dmcontrol_hasel && |hart_array_mask);
wire        dmstatus_anynonexistent     = hartsel >= N_HARTS;
wire        dmstatus_allunavail         = get_all(~hart_available);
wire        dmstatus_anyunavail         = get_any(~hart_available);
wire        dmstatus_allrunning         = get_all(hart_available & hart_running);
wire        dmstatus_anyrunning         = get_any(hart_available & hart_running);
wire        dmstatus_allhalted          = get_all(hart_available & hart_halted);
wire        dmstatus_anyhalted          = get_any(hart_available & hart_halted);
wire        dmstatus_authenticated      = 1;                // We don't support authentication            
wire        dmstatus_authbusy           = 0;                
wire        dmstatus_hasresethaltreq    = 0;                // We don't support reset halt request      // FIXME:
wire        dmstatus_confstrptrvalid    = 0;                // confstrptr valid
wire [3:0]  dmstatus_version            = 4'd2;             // Version 2: Debug Spec 0.13.2

wire [31:0] dmstatus_rval   = {9'd0, dmstatus_impebreak, 2'b00, dmstatus_allhavereset, dmstatus_anyhavereset, 
                                dmstatus_allresumeack, dmstatus_anyresumeack, dmstatus_allnonexistent, 
                                dmstatus_anynonexistent, dmstatus_allunavail, dmstatus_anyunavail, dmstatus_allrunning, 
                                dmstatus_anyrunning, dmstatus_allhalted, dmstatus_anyhalted, dmstatus_authenticated, 
                                dmstatus_authbusy, dmstatus_hasresethaltreq, dmstatus_confstrptrvalid, dmstatus_version};

////////////////////////////////////////
// 0x10: Dmcontrol register

wire                    dmcontrol_haltreq           = we_dmcontrol && dm_wb_dat_i[31];  // Request to halt all selected harts
wire                    dmcontrol_resumereq         = we_dmcontrol && dm_wb_dat_i[30];  // Request to resume all selected harts
reg                     dmcontrol_hartreset;                                            // Request to reset all selected harts    
wire                    dmcontrol_ackhavereset      = we_dmcontrol && dm_wb_dat_i[28];
reg                     dmcontrol_hasel;

reg  [HARTSELLEN-1:0]   dmcontrol_hartsel;
wire [9:0]              dmcontrol_hartselhi         = 10'd0;    // We only support 10 bits
wire [9:0]              dmcontrol_hartsello         = {{10-HARTSELLEN{1'b0}}, dmcontrol_hartsel};

wire                    dmcontrol_setresethaltreq   = we_dmcontrol && dm_wb_dat_i[3];
wire                    dmcontrol_clrresethaltreq   = we_dmcontrol && dm_wb_dat_i[2];
reg                     dmcontrol_ndmreset;

`UNUSED_VAR(dmcontrol_haltreq)
`UNUSED_VAR(dmcontrol_resumereq)
`UNUSED_VAR(dmcontrol_ackhavereset)
`UNUSED_VAR(dmcontrol_setresethaltreq)
`UNUSED_VAR(dmcontrol_clrresethaltreq)

wire [31:0] dmcontrol_rval = {
    1'b0,                           // haltreq: w only
    1'b0,                           // resumereq: w1 only
    get_any(dmcontrol_hartreset),   // hartreset
    1'b0,                           // ackhavereset: w1 only
    1'b0,                           // reserved
    dmcontrol_hasel,                // hasel
    dmcontrol_hartsello,            // hartsello
    dmcontrol_hartselhi,            // hartselhi
    2'b00,                          // reserved
    1'b0,                           // setresethaltreq: w1 only
    1'b0,                           // clrresethaltreq: w1 only
    dmcontrol_ndmreset,             // ndmreset
    dmcontrol_dmactive              // dmactive
};



always @(posedge clk_i) begin
    if (rst_i) begin
        dmcontrol_hartreset <= 1'b0;
        dmcontrol_hasel <= 1'b0;
        dmcontrol_hartsel <= 'd0;
        dmcontrol_ndmreset <= 1'b0;
        dmcontrol_dmactive <= 1'b0;
    end else begin
        if (we_dmcontrol) begin
            if (!dmcontrol_dmactive) begin                  // We can only change dmactive if dmactive is 0
                dmcontrol_dmactive <= dm_wb_dat_i[0];
                dmcontrol_ndmreset <= 1'b0;
            end
            else begin
                dmcontrol_hartreset <= dm_wb_dat_i[29];
                dmcontrol_hasel     <= dm_wb_dat_i[26];
                dmcontrol_hartsel   <= dm_wb_dat_i[16+:HARTSELLEN];      // Write hartsel-lo
                dmcontrol_ndmreset  <= dm_wb_dat_i[1];
            end
        end
    end     
end

assign ndmreset_o = dmcontrol_ndmreset;

////////////////////////////////////////
// 0x12: Hartinfo register
// - Gives information about the currently selected hart
wire [31:0] hartinfo_rval = {8'd0, 4'd0, 3'd0, 1'd0, 4'd0, 12'd0};       // TODO: check if this is correct


////////////////////////////////////////
// 0x14: Hart Array Window Select Register
// - Selects which of the 32-bit portion of hart array mask reg is accissible through hawindow register
wire [31:0] hawindowsel_rval = 32'd0;       // we don't support >32 harts


////////////////////////////////////////
// 0x15: Hart Array Window Register
// - provides R/W access to 32-bit portion of hart array mask register.
wire  [31:0]  hawindow_maskdata = {{32-N_HARTS{1'b0}}, hart_array_mask};


////////////////////////////////////////



////////////////////////////////////////
// Wishbone logic

// Reads
always @(*) /* COMBINATORIAL */ begin
    case(wb_reg_addr)
        // ADDR_DATA0:         dm_wb_dat_o = 32'h0; // We only support 1 data register
        ADDR_DMSTATUS:      dm_wb_dat_o = dmstatus_rval;
        ADDR_DMCONTROL:     dm_wb_dat_o = dmcontrol_rval;
        ADDR_HARTINFO:      dm_wb_dat_o = hartinfo_rval;
        ADDR_HAWINDOWSEL:   dm_wb_dat_o = hawindowsel_rval;
        ADDR_HAWINDOW:      dm_wb_dat_o = hawindow_maskdata;
        default:
            dm_wb_dat_o = 'dx;
    endcase
end

endmodule
