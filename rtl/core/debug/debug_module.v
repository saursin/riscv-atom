`default_nettype none

module debug_module #(
    // parameter N_HARTS       = 1,                // Number of harts
	// parameter NEXT_DM_ADDR  = 32'h0000_0000,    // Address of the next DM
    // parameter PROGBUF_SIZE  = 2,                // Number of words in the program buffer (2 words+impebreak sufficient for RV32I)
    parameter EN_IMPEBREAK = 1,

    parameter DMI_ADDRW = 32, 
    parameter DMI_DATAW = 32
)(
/* verilator lint_off UNUSEDSIGNAL */
    input wire clk_i,
    input wire rst_i,

    // output wire             ndmreset_o,


    // Debug Transport Module Ifc
    input  wire [DMI_ADDRW-1:0]    dm_wb_adr_i,
    input  wire [DMI_DATAW-1:0]    dm_wb_dat_i,
    output reg  [DMI_DATAW-1:0]    dm_wb_dat_o,
    input  wire                    dm_wb_cyc_i,
    input  wire                    dm_wb_stb_i,
    input  wire                    dm_wb_we_i,
    input  wire [DMI_DATAW/8-1:0]  dm_wb_sel_i,
    output wire                    dm_wb_err_o,
    output reg                     dm_wb_ack_o
/* verilator lint_on UNUSEDSIGNAL */
);

////////////////////////////////////////////////////////////////////////////////
// localparam ADDR_DATA0        = 6'h04;   // We only support 1 data register
localparam ADDR_DMCONTROL    = 6'h10;
localparam ADDR_DMSTATUS     = 6'h11;
// localparam ADDR_HARTINFO     = 6'h12;
// localparam ADDR_HALTSUM1     = 6'h13;
// localparam ADDR_HALTSUM0     = 6'h40;   // We only support 1 halt summary register (32 harts)
// localparam ADDR_HAWINDOWSEL  = 6'h14;
// localparam ADDR_HAWINDOW     = 6'h15;
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

/* verilator lint_off UNUSEDSIGNAL */
wire        wb_we       = dm_wb_cyc_i && dm_wb_stb_i && dm_wb_we_i;
wire        wb_re       = dm_wb_cyc_i && dm_wb_stb_i && !dm_wb_we_i;
wire [3:0]  wb_wmask    = {4{wb_we}} & dm_wb_sel_i;

wire        we_dmstatus = wb_we && (wb_reg_addr == ADDR_DMSTATUS);
wire        we_dmcontrol= wb_we && (wb_reg_addr == ADDR_DMCONTROL);
/* verilator lint_on UNUSEDSIGNAL */


////////////////////////////////////////
// Hart selection logic




////////////////////////////////////////
// 0x11: Dmstatus register

wire        dmstatus_impebreak = EN_IMPEBREAK;      // Tells debugger if impebreak is supported
wire        dmstatus_allhavereset = 0;              // All selected harts have been reset, none acknowledged reset
wire        dmstatus_anyhavereset = 0;              // Atleast 1 selected hart is reset, not acknowledged reset
wire        dmstatus_allresumeack = 0;              // All selected harts have acknowledged resume request
wire        dmstatus_anyresumeack = 0;              // Atleast 1 selected hart has acknowledged resume request
wire        dmstatus_allnonexistent = 0;            // All selected harts are nonexistent
wire        dmstatus_anynonexistent = 0;            // Atleast 1 selected hart is nonexistent
wire        dmstatus_allunavail = 0;                // All selected harts are unavailable
wire        dmstatus_anyunavail = 0;                // Atleast 1 selected hart is unavailable
wire        dmstatus_allrunning = 0;                // All selected harts are running
wire        dmstatus_anyrunning = 0;                // Atleast 1 selected hart is running
wire        dmstatus_allhalted = 0;                 // All selected harts are halted
wire        dmstatus_anyhalted = 0;                 // Atleast 1 selected hart is halted
wire        dmstatus_authenticated = 1;             // 0: auth required, 1: auth passed (We don't support auth, hardcoded to 1)
wire        dmstatus_authbusy = 0;                  // Auth is busy (We don't support auth, hardcoded to 0)
wire        dmstatus_hasresethaltreq = 0;           // DM supports halt-on-reset (currently not supported)
wire        dmstatus_confstrptrvalid = 0;           // Config string pointer is valid (currently not supported)
wire [3:0]  dmstatus_version = 4'd2;                // Debug spec version (0: version 0.11, 1: version 0.13, 15: none)

wire [31:0] dmstatus_readval = {9'd0, dmstatus_impebreak, 2'b00, dmstatus_allhavereset, dmstatus_anyhavereset, 
                                dmstatus_allresumeack, dmstatus_anyresumeack, dmstatus_allnonexistent, 
                                dmstatus_anynonexistent, dmstatus_allunavail, dmstatus_anyunavail, dmstatus_allrunning, 
                                dmstatus_anyrunning, dmstatus_allhalted, dmstatus_anyhalted, dmstatus_authenticated, 
                                dmstatus_authbusy, dmstatus_hasresethaltreq, dmstatus_confstrptrvalid, dmstatus_version};

////////////////////////////////////////
// 0x10: Dmcontrol register

wire        dmcontrol_haltreq      = we_dmcontrol && dm_wb_dat_i[31];           // Request to halt all selected harts
wire        dmcontrol_resumereq    = we_dmcontrol && dm_wb_dat_i[30];           // Request to resume all selected harts
reg         dmcontrol_hartreset;                                                // Request to reset all selected harts    
wire        dmcontrol_ackhavereset = we_dmcontrol && dm_wb_dat_i[28];
reg         dmcontrol_hasel;
reg  [9:0]  dmcontrol_hartsello;
reg  [9:0]  dmcontrol_hartselhi;
wire        dmcontrol_setresethaltreq = we_dmcontrol && dm_wb_dat_i[3];
wire        dmcontrol_clrresethaltreq = we_dmcontrol && dm_wb_dat_i[2];
reg         dmcontrol_ndmreset;
reg         dmcontrol_dmactive;

`UNUSED_VAR(dmcontrol_haltreq)
`UNUSED_VAR(dmcontrol_resumereq)
`UNUSED_VAR(dmcontrol_ackhavereset)
`UNUSED_VAR(dmcontrol_setresethaltreq)
`UNUSED_VAR(dmcontrol_clrresethaltreq)


wire [31:0] dmcontrol_readval = {1'b0, 1'b0, dmcontrol_hartreset, 1'b0, 1'b0, dmcontrol_hasel, dmcontrol_hartsello, 
                                dmcontrol_hartselhi, 2'b00, 1'b0, 1'b0, dmcontrol_ndmreset, dmcontrol_dmactive};

always @(posedge clk_i) begin
    if (rst_i) begin
        dmcontrol_hartreset <= 1'b0;
        dmcontrol_hasel <= 1'b0;
        dmcontrol_hartsello <= 10'd0;
        dmcontrol_hartselhi <= 10'd0;
        dmcontrol_ndmreset <= 1'b0;
        dmcontrol_dmactive <= 1'b0;
    end else begin
        if (we_dmcontrol) begin
            dmcontrol_hartreset <= dm_wb_dat_i[29];
            dmcontrol_hasel <= dm_wb_dat_i[26];
            dmcontrol_hartsello <= dm_wb_dat_i[25:16];
            dmcontrol_hartselhi <= dm_wb_dat_i[15:6];
            dmcontrol_ndmreset <= dm_wb_dat_i[1];
            dmcontrol_dmactive <= dm_wb_dat_i[0];
        end
    end     
end




////////////////////////////////////////
// Wishbone logic

// Reads
always @(*) /* COMBINATORIAL */ begin
    case(wb_reg_addr)
        ADDR_DMSTATUS:        dm_wb_dat_o = dmstatus_readval;
        ADDR_DMCONTROL:       dm_wb_dat_o = dmcontrol_readval;
        default:
            dm_wb_dat_o = 'dx;
    endcase
end

endmodule
