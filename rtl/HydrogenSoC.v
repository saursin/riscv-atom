`default_nettype none

`include "Timescale.vh"
`include "Config.vh"
`include "uncore/SinglePortROM_wb.v"
`include "uncore/SinglePortRAM_wb.v"
`include "core/AtomRV_wb.v"

/**
 *  Hydogen SoC
 *  Barebone SoC housing a single atom core, imem, & dmem.
 */

module HydrogenSoC
(
    input clk_i,
    input rst_i
);
    reg wb_clk_i = 0;
    reg wb_rst_i = 0;


    //////////////////////////////////////////
    // Signals

    wire    [31:0]  wb_ibus_adr_o;
    wire    [31:0]  wb_ibus_dat_i;

    wire            wb_ibus_ack_i;
    wire            wb_ibus_stb_o;

    wire    [31:0]  wb_dbus_adr_o;
    wire    [31:0]  wb_dbus_dat_o;
    wire    [31:0]  wb_dbus_dat_i;
    
    wire            wb_dbus_we_o;
    wire    [3:0]   wb_dbus_sel_o;

    wire            wb_dbus_stb_o;
    wire            wb_dbus_ack_i;
    wire            wb_dbus_cyc_o;


    ////////////////////////////////////////////////////
    // Instruction Memory
    SinglePortROM_wb #(
    .ADDR_WIDTH(16),
    .MEM_INIT_FILE("rtl/IMEM_INIT_FILE.txt")
    ) imem
    (
        .clk_i      (wb_clk_i),
        .rst_i      (wb_rst_i),

        .addr_i     (wb_ibus_adr_o[15:2]),
        .data_o     (wb_ibus_dat_i),

        .stb_i      (wb_ibus_stb_o),
        .ack_o      (wb_ibus_ack_i)
    );


    ////////////////////////////////////////////////////
    // Data Memory
    SinglePortRAM #(
    .ADDR_WIDTH(16),
    .MEM_FILE("rtl/DMEM_INIT_FILE.txt")
    ) dmem 
    (
    // Wishbone Interface
        .wb_clk_i   (wb_clk_i),
        .wb_rst_i   (wb_rst_i),

        .wb_adr_i   (wb_dbus_adr_o[15:2]),
        .wb_dat_o   (wb_dbus_dat_i),
        .wb_dat_i   (wb_dbus_dat_o),
        .wb_we_i    (wb_dbus_we_o),
        .wb_sel_i   (wb_dbus_sel_o),

        .wb_stb_i   (wb_dbus_stb_o),
        .wb_ack_o   (wb_dbus_ack_i),
        .wb_cyc_i   (wb_dbus_cyc_o)
    );


    /////////////////////////////////////////////////
    // Atom Wishbone Core
    AtomRV_wb atom_core
    (   
        .wb_clk_i       (wb_clk_i),
        .wb_rst_i       (wb_rst_i),

        // === IBUS Wishbone Master Interface ===
        .wb_ibus_adr_o  (wb_ibus_adr_o),
        .wb_ibus_dat_i  (wb_ibus_dat_i),

        .wb_ibus_stb_o  (wb_ibus_stb_o),
        .wb_ibus_ack_i  (wb_ibus_ack_i),
         
        // === DBUS Wishbone Master Interface ===
        .wb_dbus_adr_o  (wb_dbus_adr_o),
        .wb_dbus_dat_o  (wb_dbus_dat_o),
        .wb_dbus_dat_i  (wb_dbus_dat_i),
        
        .wb_dbus_we_o   (wb_dbus_we_o),
        .wb_dbus_sel_o  (wb_dbus_sel_o),

        .wb_dbus_stb_o  (wb_dbus_stb_o),
        .wb_dbus_ack_i  (wb_dbus_ack_i),
        .wb_dbus_cyc_o  (wb_dbus_cyc_o)

        // === IRQ Interface ===
        //input   wire    [31:0]  irq,
        //output  reg     [31:0]  eoi
    );


    initial begin
        $dumpfile("out.vcd");
        $dumpvars(0, HydrogenSoC);
        
        // Global reset
        wb_rst_i = 1'b1;
        #4;
        wb_rst_i = 1'b0;
        #4;
        
        #1000;
        $finish();
    end

    always begin
        #1;
        wb_clk_i = !wb_clk_i;
    end

endmodule