`default_nettype none

module debug_subsystem #(
    parameter TAP_IDCODE            = 32'h0000_0001,    // JTAG IDCODE {31:28-version, 27-12-partNumber, 21:1-manufId, 1}
    parameter DMI_ADDRW             = 8,                // DMI address width
    parameter DMI_DATAW             = 32,               // DMI data width
    parameter DTMCS_IDLE_HINT       = 3'd4,             // Number of cycles debugger should spend in idle state to avoid dmistat == 3
    parameter DEBUG_SPEC_VERSION    = 4'b0001           // 0: version 0.11, 1: version 0.13, 15: none  
)(
    input   wire    clk_i,
    input   wire    rst_i,
    
    input   wire    jtag_tck_i,
    input   wire    jtag_trst_n_i,
    input   wire    jtag_tms_i,
    input   wire    jtag_tdi_i,
    output  wire    jtag_tdo_o
);   
   
    ////////////////////////////////////////////////////////////////////////////
    // Debug Transport Module

    /* verilator lint_off UNUSED */
    wire [DMI_ADDRW-1:0]    dmi_wb_adr_o;
    wire [DMI_DATAW-1:0]    dmi_wb_dat_o;
    wire [DMI_DATAW-1:0]    dmi_wb_dat_i;
    wire                    dmi_wb_cyc_o;
    wire                    dmi_wb_stb_o;
    wire                    dmi_wb_we_o;
    wire [DMI_DATAW/8-1:0]  dmi_wb_sel_o;
    wire                    dmi_wb_err_i;
    wire                    dmi_wb_ack_i;
    /* verilator lint_on UNUSED */

    debug_transport_module #(
        .TAP_IDCODE(TAP_IDCODE),
        .DMI_ADDRW(DMI_ADDRW),
        .DMI_DATAW(DMI_DATAW),
        .DTMCS_IDLE_HINT(DTMCS_IDLE_HINT),
        .DEBUG_SPEC_VERSION(DEBUG_SPEC_VERSION)
    ) dtm (
        .tck_i              (jtag_tck_i),
        .trst_n_i           (jtag_trst_n_i),
        .tms_i              (jtag_tms_i),
        .tdi_i              (jtag_tdi_i),
        .tdo_o              (jtag_tdo_o),
        .dmi_wb_adr_o       (dmi_wb_adr_o),
        .dmi_wb_dat_o       (dmi_wb_dat_o),
        .dmi_wb_dat_i       (dmi_wb_dat_i),
        .dmi_wb_cyc_o       (dmi_wb_cyc_o),
        .dmi_wb_stb_o       (dmi_wb_stb_o),
        .dmi_wb_we_o        (dmi_wb_we_o),
        .dmi_wb_sel_o       (dmi_wb_sel_o),
        .dmi_wb_err_i       (dmi_wb_err_i),
        .dmi_wb_ack_i       (dmi_wb_ack_i)
    );


    ////////////////////////////////////////////////////////////////////////////
    // Clock Domain Crossing
    wire [DMI_ADDRW-1:0]    dm_wb_adr_o;
    wire [DMI_DATAW-1:0]    dm_wb_dat_o;
    wire [DMI_DATAW-1:0]    dm_wb_dat_i;
    wire                    dm_wb_cyc_o;
    wire                    dm_wb_stb_o;
    wire                    dm_wb_we_o;
    wire [DMI_DATAW/8-1:0]  dm_wb_sel_o;
    wire                    dm_wb_err_i;
    wire                    dm_wb_ack_i;


    async_reg_wb #(
        .DATA_WIDTH(DMI_DATAW),
        .ADDR_WIDTH(DMI_ADDRW)
    ) dmi_cdc (
        // master side
        .wbm_clk    (jtag_tck_i),
        .wbm_rst    (!jtag_trst_n_i),
        .wbm_adr_i  (dmi_wb_adr_o),
        .wbm_dat_i  (dmi_wb_dat_o),
        .wbm_dat_o  (dmi_wb_dat_i),
        .wbm_we_i   (dmi_wb_we_o),
        .wbm_sel_i  (dmi_wb_sel_o),
        .wbm_stb_i  (dmi_wb_stb_o),
        .wbm_ack_o  (dmi_wb_ack_i),
        .wbm_err_o  (dmi_wb_err_i),
        /* verilator lint_off PINCONNECTEMPTY */
        .wbm_rty_o  (),
        /* verilator lint_on PINCONNECTEMPTY */
        .wbm_cyc_i  (dmi_wb_cyc_o),

        // slave side
        .wbs_clk    (clk_i),
        .wbs_rst    (rst_i),
        .wbs_adr_o  (dm_wb_adr_o),
        .wbs_dat_i  (dm_wb_dat_i),
        .wbs_dat_o  (dm_wb_dat_o),
        .wbs_we_o   (dm_wb_we_o),
        .wbs_sel_o  (dm_wb_sel_o),
        .wbs_stb_o  (dm_wb_stb_o),
        .wbs_ack_i  (dm_wb_ack_i),
        .wbs_err_i  (dm_wb_err_i),
        /* verilator lint_off PINCONNECTEMPTY */
        .wbs_rty_i  (),
        /* verilator lint_on PINCONNECTEMPTY */
        .wbs_cyc_o  (dm_wb_cyc_o) 
    );


    ////////////////////////////////////////////////////////////////////////////
    // Debug Module

    debug_module #(
        .DMI_ADDRW(DMI_ADDRW),
        .DMI_DATAW(DMI_DATAW)
    ) dm (
        .clk_i         (clk_i),
        .rst_i         (rst_i),
        .dm_wb_adr_i   (dm_wb_adr_o),
        .dm_wb_dat_i   (dm_wb_dat_o),
        .dm_wb_dat_o   (dm_wb_dat_i),
        .dm_wb_cyc_i   (dm_wb_cyc_o),
        .dm_wb_stb_i   (dm_wb_stb_o),
        .dm_wb_we_i    (dm_wb_we_o),
        .dm_wb_sel_i   (dm_wb_sel_o),
        .dm_wb_err_o   (dm_wb_err_i),
        .dm_wb_ack_o   (dm_wb_ack_i)
    );

endmodule
