////////////////////////////////////////////////////////////////////   
//  File        : AtomRV_wb.v
//  Author      : Saurabh Singh (saurabh.s99100@gmail.com)
//  Description : Wishbone wrapper for Atom core
////////////////////////////////////////////////////////////////////
`default_nettype none

/*
    Wishbone wrapper for the atom cpu.
*/
module AtomRV_wb
(
    input   wire            wb_clk_i,
    input   wire            wb_rst_i,

    input   wire    [31:0]  reset_vector_i,

    // === IBUS Wishbone Master Interface ===
    output  reg     [31:0]  iport_wb_adr_o,
    input   wire    [31:0]  iport_wb_dat_i,
    output  reg             iport_wb_cyc_o,
    output  reg             iport_wb_stb_o,
    input   wire            iport_wb_ack_i,


    // === DBUS Wishbone Master Interface === 
    output  reg     [31:0]  dport_wb_adr_o,
    output  reg     [31:0]  dport_wb_dat_o,
    input   wire    [31:0]  dport_wb_dat_i,
    output  reg             dport_wb_cyc_o,
    output  reg             dport_wb_stb_o,
    output  reg             dport_wb_we_o,
    output  reg     [3:0]   dport_wb_sel_o,  
    input   wire            dport_wb_ack_i

    `ifdef EN_EXCEPT
    // Interrupt Signals
    ,
    input   wire            irq_i,
    input   wire            timer_int_i
    `endif // EN_EXCEPT

    `ifdef EN_DEBUG
    // Debug Signals
    ,
    input   wire            jtag_tck_i,
    input   wire            jtag_trst_n_i,
    input   wire            jtag_tms_i,
    input   wire            jtag_tdi_i,
    output  wire            jtag_tdo_o
    `endif // EN_DEBUG
);
    /////////////////////////////////////////////////////////////////
    wire    [31:0]  iport_addr_o;    // IMEM Address
    wire    [31:0]  iport_data_i;    // IMEM data

    wire            iport_valid_o;   // IMEM valid
    wire            iport_ack_i;     // IMEM Acknowledge
    

    wire    [31:0]  dport_addr_o;    // DMEM address
    wire    [31:0]  dport_data_i;    // DMEM data in
    wire    [31:0]  dport_data_o;    // DMEM data out

    wire    [3:0]   dport_sel_o;     // DMEM Access width
    wire            dport_we_o;      // DMEM Access width

    wire            dport_valid_o;   // DMEM Access width
    wire            dport_ack_i;     // DMEM WriteEnable

    
    // Atom Core
    AtomRV atom_core
    (
        .clk_i          (wb_clk_i),
        .rst_i          (wb_rst_i),

        .reset_vector_i (reset_vector_i),

        .iport_addr_o    (iport_addr_o),
        .iport_data_i    (iport_data_i),
        .iport_valid_o   (iport_valid_o),
        .iport_ack_i     (iport_ack_i),


        .dport_addr_o    (dport_addr_o),
        .dport_data_i    (dport_data_i),
        .dport_data_o    (dport_data_o),
        .dport_sel_o     (dport_sel_o),
        .dport_we_o      (dport_we_o),
        .dport_valid_o   (dport_valid_o),
        .dport_ack_i     (dport_ack_i) 

        `ifdef EN_EXCEPT
        ,
        .irq_i          (irq_i),
        .timer_int_i    (timer_int_i)
        `endif // EN_EXCEPT

        `ifdef EN_DEBUG
        ,
        .jtag_tck_i     (jtag_tck_i),
        .jtag_trst_n_i  (jtag_trst_n_i),
        .jtag_tms_i     (jtag_tms_i),
        .jtag_tdi_i     (jtag_tdi_i),
        .jtag_tdo_o     (jtag_tdo_o)
        `endif // EN_DEBUG
    );
    

	localparam WBIDLE = 1'b0;
	localparam WBACTIV = 1'b1;

    ////////////////////////////////////////////////////////////
    /// IPORT Wishbone Logic
    assign iport_ack_i = iport_wb_ack_i;
    assign iport_data_i = iport_wb_dat_i;

    reg iport_state = WBIDLE;

    always @(posedge wb_clk_i) begin
        if(wb_rst_i) begin
            iport_wb_adr_o <= 0;
            iport_wb_cyc_o <= 0;
            iport_wb_stb_o <= 0;
            iport_state <= WBIDLE;
        end
        else begin
            case(iport_state)
                WBIDLE: begin
                    if(iport_valid_o) begin
                        iport_wb_adr_o <= iport_addr_o;
                        iport_wb_cyc_o <= 1'b1;
                        iport_wb_stb_o <= 1'b1;
                        iport_state <= WBACTIV;
                    end else begin
                        iport_wb_cyc_o <= 1'b0;
                        iport_wb_stb_o <= 1'b0;
                    end
                end
                WBACTIV: begin
                    if(iport_wb_ack_i) begin
                        iport_wb_cyc_o <= 1'b0;
                        iport_wb_stb_o <= 1'b0;
                        iport_state <= WBIDLE;
                    end
                end
                default:
                    iport_state <= WBIDLE;
            endcase
        end
    end


    ////////////////////////////////////////////////////////////
    /// DPORT Wishbone Logic
    assign dport_ack_i = dport_wb_ack_i;
    assign dport_data_i = dport_wb_dat_i;

    reg dport_state = WBIDLE;

    always @(posedge wb_clk_i) begin
        if(wb_rst_i) begin
            dport_wb_adr_o <= 32'd00000000;
            dport_wb_dat_o <= 32'h00000000;
            dport_wb_cyc_o <= 1'b0;
            dport_wb_stb_o <= 1'b0;
            dport_wb_we_o  <= 1'b0;
            dport_wb_sel_o <= 4'b0000;
            dport_state <= WBIDLE;
        end
        else begin
            case(dport_state)
                WBIDLE: begin
                    if(dport_valid_o) begin
                        dport_wb_adr_o <= dport_addr_o;
                        dport_wb_dat_o <= dport_data_o;
                        dport_wb_we_o <= dport_we_o;
                        dport_wb_sel_o <= dport_sel_o;
                        dport_wb_cyc_o <= 1'b1;
                        dport_wb_stb_o <= 1'b1;
                        dport_state <= WBACTIV;
                    end else begin
                        dport_wb_cyc_o <= 1'b0;
                        dport_wb_stb_o <= 1'b0;
                        dport_wb_we_o <= 1'b0;
                    end
                end
                WBACTIV: begin
                    if(dport_wb_ack_i) begin
                        dport_wb_cyc_o <= 1'b0;
                        dport_wb_stb_o <= 1'b0;
                        dport_wb_we_o <= 1'b0;
                        dport_state <= WBIDLE;
                    end
                end
                default:
                    dport_state <= WBIDLE;
            endcase
        end
    end
endmodule
