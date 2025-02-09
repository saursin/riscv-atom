`default_nettype none

module debug_subsystem #(
    parameter TAP_IDCODE            = 32'h0000_0001,    // JTAG IDCODE {31:28-version, 27-12-partNumber, 21:1-manufId, 1}
    parameter DMI_ADDRW             = 9,                // DMI address width
    parameter DMI_DATAW             = 32,               // DMI data width
    parameter DTMCS_IDLE_HINT       = 3'd4,             // Number of cycles debugger should spend in idle state to avoid dmistat == 3
    parameter DEBUG_SPEC_VERSION    = 4'b0001           // 0: version 0.11, 1: version 0.13, 15: none  
)(
    input   wire      jtag_tck_i,
    input   wire      jtag_trst_n_i,
    input   wire      jtag_tms_i,
    input   wire      jtag_tdi_i,
    output  wire      jtag_tdo_o
);   
   
    ////////////////////////////////////////////////////////////////////////////
    // Debug Transport Module

    /* verilator lint_off UNUSED */
    wire [DMI_ADDRW-1:0]    dmi_wb_adr_o;
    wire [DMI_DATAW-1:0]    dmi_wb_dat_o;
    wire [DMI_DATAW-1:0]    dmi_wb_dat_i = 0;
    wire                    dmi_wb_cyc_o;
    wire                    dmi_wb_stb_o;
    wire                    dmi_wb_we_o;
    wire [DMI_DATAW/8-1:0]  dmi_wb_sel_o;
    wire                    dmi_wb_ack_i = 0;
    /* verilator lint_on UNUSED */

    dtm #(
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
        .dmi_wb_ack_i       (dmi_wb_ack_i)
    );
endmodule
