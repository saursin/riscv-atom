`default_nettype none
`include "AtomRV.v"

/*
    Wishbone wrapper for the atom cpu.
*/
module AtomRV_wb
(
   	input   wire            wb_clk_i,
    input   wire            wb_rst_i,

	// === IBUS Wishbone Master Interface ===
	output  wire    [31:0]  wb_ibus_adr_o,
    input   wire    [31:0]  wb_ibus_dat_i,

    output  wire             wb_ibus_stb_o,
	input   wire            wb_ibus_ack_i,

    // === DBUS Wishbone Master Interface === 
    output  wire    [31:0]  wb_dbus_adr_o,
    output  wire    [31:0]  wb_dbus_dat_o,
    input   wire    [31:0]  wb_dbus_dat_i,
    
    output  wire            wb_dbus_we_o,
    output  wire    [3:0]   wb_dbus_sel_o,

    output  wire            wb_dbus_stb_o,
    input   wire            wb_dbus_ack_i,
    output  wire            wb_dbus_cyc_o

    // === IRQ Interface ===
    //input   wire    [31:0]  irq,
    //output  reg     [31:0]  eoi
);
    /////////////////////////////////////////////////////////////////
    wire    [31:0]  imem_addr_o;              // IMEM Address
	wire    [31:0]  imem_data_i;              // IMEM data

	wire       		imem_valid_o;    			// DMEM Access width
    wire            imem_ack_i;               // DMEM WriteEnable
    
    wire    [31:0]  dmem_addr_o;            // DMEM address
    wire    [31:0]  dmem_data_i;            // DMEM data in
    wire    [31:0]  dmem_data_o;            // DMEM data out

    wire    [3:0]   dmem_sel_o;    			// DMEM Access width
	wire       		dmem_we_o;    			// DMEM Access width

	wire       		dmem_valid_o;    			// DMEM Access width
    wire            dmem_ack_i;               // DMEM WriteEnable

    // Core
	AtomRV atom_core
	(
		.clk_i			(wb_clk_i),
		.rst_i			(wb_rst_i),

		.imem_addr_o	(imem_addr_o),
		.imem_data_i	(imem_data_i),

		.imem_valid_o	(imem_valid_o),
		.imem_ack_i		(imem_ack_i),



		.dmem_addr_o	(dmem_addr_o),
		.dmem_data_i	(dmem_data_i),
		.dmem_data_o	(dmem_data_o),

		.dmem_sel_o		(dmem_sel_o),
		.dmem_we_o		(dmem_we_o),

		.dmem_valid_o	(dmem_valid_o),
		.dmem_ack_i		(dmem_ack_i) 
	);
	
	////////////////////////////////////////////////////////////
	/// IBUS Wishbone Logic
	// parameter IDLE = 0;
	// parameter WAIT = 1;

	// reg state = IDLE;

	// always @(posedge wb_clk_i) begin
	// 	wb_ibus_stb_o <= 0;
	// 	case(state)
	// 		IDLE:	if(imem_valid_o) begin
	// 					wb_ibus_stb_o <= 1;
	// 					state <= WAIT;
	// 				end

	// 		WAIT:	if(imem_ack_i)
	// 					state <= IDLE;

	// 	endcase
	// end

	assign wb_ibus_adr_o = imem_addr_o;
	assign imem_data_i = wb_ibus_dat_i;
	assign wb_ibus_stb_o = imem_valid_o;
	assign imem_ack_i = wb_ibus_ack_i;

	////////////////////////////////////////////////////////////
	/// DBUS Wishbone Logic
	assign wb_dbus_adr_o = dmem_addr_o;
	assign dmem_data_i = wb_dbus_dat_i;
	assign wb_dbus_dat_o = dmem_data_o;

	assign wb_dbus_sel_o = dmem_sel_o;
	assign wb_dbus_we_o = dmem_we_o;

	assign wb_dbus_stb_o = dmem_valid_o;
	assign dmem_ack_i = wb_dbus_ack_i;

	assign wb_dbus_cyc_o = wb_dbus_stb_o;
endmodule