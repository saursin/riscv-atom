`define RG_SCKDIV_OFFSET   3'b000       // 0x00
`define RG_SCTRL_OFFSET    3'b001       // 0x04
`define RG_TDATA_OFFSET    3'b010       // 0x08
`define RG_RDATA_OFFSET    3'b011       // 0x0c
`define RG_CSCTRL_OFFSET   3'b100       // 0x10
`define RG_DCTRL_OFFSET    3'b101       // 0x14

`default_nettype none

module SPI_wb#(
    parameter NCS = 4,
    parameter DEFAULT_SCKDIV = 1,
    parameter DEFAULT_DCTRL_PRECSHD = 1,
    parameter DEFAULT_DCTRL_POSCSLD  = 1
) (
    // Wishbone Interface
    input   wire                wb_clk_i,
    input   wire                wb_rst_i,

    input   wire    [4:2]       wb_adr_i,
    output  reg     [31:0]      wb_dat_o,
    input   wire    [31:0]      wb_dat_i,
    input   wire                wb_we_i,
    input   wire    [3:0]       wb_sel_i,
    input   wire                wb_stb_i,
    output  reg                 wb_ack_o,

    output  wire                sck_o,
    input   wire                miso_i,
    output  wire                mosi_o,
    output  reg     [NCS-1:0]   cs_o
);

// Set Ack_o
always @(posedge wb_clk_i) begin
  if (wb_rst_i)
    wb_ack_o <= 1'b0;
  else
    wb_ack_o <= wb_stb_i & !wb_ack_o;
end

wire    [3:0]   we  = {4{wb_we_i & wb_stb_i}} & wb_sel_i;

// Reg: SCKDIV  (SCK Divisor Register)
reg  [31:0]     rg_sckdiv_div;
wire [31:0]     rg_sckdiv_readval = rg_sckdiv_div;

// Reg: SCTRL   (Status and Control Register)

reg             rg_sctrl_loopen;
reg             rg_sctrl_end;
reg             rg_sctrl_pha;
reg             rg_sctrl_pol;
reg             rg_sctrl_spien;

wire            spi_busy = ~core_cs_o;
wire [31:0]     rg_sctrl_readval = {spi_busy, 27'd0, rg_sctrl_end, rg_sctrl_pha, rg_sctrl_pol, rg_sctrl_spien};

// Reg: TDATA   (Transmit Data register)
wire [31:0]     rg_tdata_readval = 32'd0;
wire rg_tdata_we = (wb_adr_i == `RG_TDATA_OFFSET) && wb_we_i && wb_stb_i && wb_sel_i[0];

// Reg: RDATA   (Recieve Data register)
wire [7:0]      rg_rdata_data;
wire [31:0]     rg_rdata_readval = {24'd0, rg_rdata_data};

// Reg: CSCTRL  (Chip Select Control Register)
reg  [NCS-1:0]  rg_csctrl_acs;
reg  [1:0]      rg_csctrl_csmode;
wire [31:0]     rg_csctrl_readval = { {(8-NCS){1'b0}}, rg_csctrl_acs, 22'd0, rg_csctrl_csmode};

// Reg: DCTRL    (Delay Control Register)
reg  [7:0]      rg_dctrl_prcshd;
reg  [7:0]      rg_dctrl_pocsld;
wire [31:0]     rg_dctrl_readval = {16'd0, rg_dctrl_prcshd, rg_dctrl_pocsld};

// Handle Writes
always @(posedge wb_clk_i) begin
    if(wb_rst_i) begin
        rg_sckdiv_div       <= DEFAULT_SCKDIV;

        rg_sctrl_loopen     <= 1'b0;
        rg_sctrl_end        <= 1'b0;
        rg_sctrl_pha        <= 1'b0;
        rg_sctrl_pol        <= 1'b0;
        rg_sctrl_spien      <= 1'b0;

        rg_csctrl_acs       <= 1;
        rg_csctrl_csmode    <= 2'b00;

        rg_dctrl_prcshd     <= DEFAULT_DCTRL_PRECSHD;
        rg_dctrl_pocsld     <= DEFAULT_DCTRL_POSCSLD;
    end
    else begin
        case(wb_adr_i)
            `RG_SCKDIV_OFFSET: begin
                if (we[0])  rg_sckdiv_div[7:0]      <= wb_dat_i[7:0];
                if (we[1])  rg_sckdiv_div[15:8]     <= wb_dat_i[15:8];
                if (we[2])  rg_sckdiv_div[23:16]    <= wb_dat_i[23:16];
                if (we[3])  rg_sckdiv_div[31:24]    <= wb_dat_i[31:24];
            end

            `RG_SCTRL_OFFSET: begin
                if(we[0])   {rg_sctrl_loopen, rg_sctrl_end, rg_sctrl_pha, rg_sctrl_pol, rg_sctrl_spien}  <= wb_dat_i[4:0];
            end

            `RG_CSCTRL_OFFSET: begin
                if(we[0])   rg_csctrl_csmode    <= wb_dat_i[1:0];
                if(we[3])   rg_csctrl_acs       <= wb_dat_i[24+NCS-1:24];
            end

            `RG_DCTRL_OFFSET: begin
                if(we[0])   rg_dctrl_pocsld     <= wb_dat_i[7:0];
                if(we[1])   rg_dctrl_prcshd     <= wb_dat_i[15:8];
            end

            default: begin
            end
        endcase
    end
end


// READS
always @(*) /* COMBINATORIAL */ begin
    case(wb_adr_i)
        `RG_SCKDIV_OFFSET:  wb_dat_o = rg_sckdiv_readval;
        `RG_SCTRL_OFFSET:   wb_dat_o = rg_sctrl_readval;
        `RG_TDATA_OFFSET:   wb_dat_o = rg_tdata_readval;
        `RG_RDATA_OFFSET:   wb_dat_o = rg_rdata_readval;
        `RG_CSCTRL_OFFSET:  wb_dat_o = rg_csctrl_readval;
        `RG_DCTRL_OFFSET:   wb_dat_o = rg_dctrl_readval;
        default:            wb_dat_o = 32'dx;
    endcase
end


// CS logic
always @(*) /* COMBINATORIAL */ begin
    case(rg_csctrl_csmode)
        2'b00:      cs_o = rg_csctrl_acs & {NCS{core_cs_o}};
        2'b11:      cs_o = ~rg_csctrl_acs;
        default:    cs_o = {NCS{1'b1}};
    endcase
end

wire core_cs_o;
SPI_core spi_core (
    .clk_i          (wb_clk_i),
    .rst_i          (wb_rst_i),
    .spi_en_i       (rg_sctrl_spien),
    .lsb_first_i    (rg_sctrl_end),
    .pol_i          (rg_sctrl_pol),
    .pha_i          (rg_sctrl_pha),
    .post_csl_delay (rg_dctrl_pocsld),
    .pre_csh_delay  (rg_dctrl_prcshd),
    .txdata_i       (wb_dat_i[7:0]),
    .tx_we_i        (rg_tdata_we),
    .rxdata_o       (rg_rdata_data),
    .sckdiv_i       (rg_sckdiv_div),
    .sck_o          (sck_o),
    .miso_i         (rg_sctrl_loopen ? mosi_o : miso_i),
    .mosi_o         (mosi_o),
    .cs_o           (core_cs_o)
);

endmodule
