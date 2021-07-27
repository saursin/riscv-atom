`default_nettype none

`include "Timescale.vh"
`include "Config.vh"
`include "uncore/SinglePortROM_wb.v"
`include "uncore/SinglePortRAM_wb.v"
`include "core/AtomRV_wb.v"

/**
 *  === Hydrogen SoC ===
 *  Barebone SoC housing a single atom core, instruction memory, data memory and a uart slave.
 */

module HydrogenSoC
(
    input clk_i,
    input rst_i
);
    //////////////////////////////////////////
    // SoC Parameters

    parameter IMEM_ADR_SIZE = 15;   // 32KB
    parameter DMEM_ADR_SIZE = 13;   // 8KB

    //////////////////////////////////////////
    // Global Bus Signals

    wire wb_clk_i = clk_i;
    wire wb_rst_i = rst_i;

    /* verilator lint_off UNUSED */
    wire    [31:0]  wb_ibus_adr_o;
    /* verilator lint_on UNUSED */

    wire    [31:0]  wb_ibus_dat_i;
    wire            wb_ibus_ack_i;
    wire            wb_ibus_stb_o;


    wire    [31:0]  wb_dbus_adr_o   /* verilator public */;
    wire    [31:0]  wb_dbus_dat_o   /* verilator public */;
    reg     [31:0]  wb_dbus_dat_i   /* verilator public */;    
    wire            wb_dbus_we_o    /* verilator public */;
    wire    [3:0]   wb_dbus_sel_o   /* verilator public */;
    wire            wb_dbus_stb_o   /* verilator public */;
    reg             wb_dbus_ack_i   /* verilator public */;
    
    /* verilator lint_off UNUSED */
    wire            wb_dbus_cyc_o   /* verilator public */;
    /* verilator lint_on UNUSED */



    /////////////////////////////////////////////////
    // Atom Wishbone Core
    AtomRV_wb atom_wb_core
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



    ////////////////////////////////////////////////////
    // Instruction Memory

    SinglePortROM_wb #(
        .ADDR_WIDTH(IMEM_ADR_SIZE),
        .MEM_INIT_FILE("build/init/code.hex")
    ) imem
    (
        .clk_i      (wb_clk_i),
        .rst_i      (wb_rst_i),

        .addr_i     (wb_ibus_adr_o[IMEM_ADR_SIZE-1:2]),
        .data_o     (wb_ibus_dat_i),

        .stb_i      (wb_ibus_stb_o),
        .ack_o      (wb_ibus_ack_i)
    );


    ////////////////////////////////////////////////////
    // Data Memory

    wire    [31:0] wb_ram_data_o; 
    reg            wb_ram_stb_i;
    reg            wb_ram_ack_o;

    SinglePortRAM_wb #(
        .ADDR_WIDTH(DMEM_ADR_SIZE),
        .MEM_FILE("build/init/data.hex")
    ) dmem 
    (
        .wb_clk_i   (wb_clk_i),
        .wb_rst_i   (wb_rst_i),

        .wb_adr_i   (wb_dbus_adr_o[DMEM_ADR_SIZE-1:2]),
        .wb_dat_o   (wb_ram_data_o),
        .wb_dat_i   (wb_dbus_dat_o),
        .wb_we_i    (wb_dbus_we_o),
        .wb_sel_i   (wb_dbus_sel_o),

        .wb_stb_i   (wb_ram_stb_i),
        .wb_ack_o   (wb_ram_ack_o)
    );



    //////////////////////////////////////////////////
    // UART
    wire    [31:0]  wb_uart_data_o  /* verilator public */;
    reg             wb_uart_stb_i   /* verilator public */;
    reg             wb_uart_ack_o   /* verilator public */;




    ////////////////////////////////////////////////////
    // Wishbone Interconnect Logic

    // Devices
    localparam Device_None = 3'b000;
    localparam Device_RAM  = 3'b001;
    localparam Device_UART = 3'b010;

    /*
        === Device selection ===
        A device is selected among available devices, depending on which region of memory 
        does the cpu wants to access. If none of the device is selected (i.e. cpu tries to 
        access a region of memory which is not mapped to any device) an errror is thrown &
        simulation is haulted.
    */
    reg [2:0] selected_device /* verilator public */;
    always @(*) begin /* COMBINATORIAL */
        if(wb_dbus_cyc_o) begin
            if(wb_dbus_adr_o >= 32'h04000000 && wb_dbus_adr_o < 32'h08000000)
                selected_device = Device_RAM;
            else if (wb_dbus_adr_o == 32'h08000000) // byte addresses 8000000 to 08000003
                selected_device = Device_UART;
            else begin
                $display("RTL-ERROR: Unknown Device Selected: 0x%x\nHaulting simulation...", wb_dbus_adr_o);
                $finish();
            end
        end
        else begin
            selected_device = Device_None;
        end
    end

    /*
        === Data Muxing ===
        After device selection, data from the selected device is muxed to the wb_dat_i port 
        of the cpu.
    */
    always @(*) begin /* COMBINATORIAL */
        case(selected_device)
            Device_RAM:     wb_dbus_dat_i = wb_ram_data_o;
            Device_UART:    wb_dbus_dat_i = wb_uart_data_o;

            default: begin
                wb_dbus_dat_i = 32'h00000000;
            end
        endcase
    end

    /*
        === Stb muxing ===
        In order to initiate a transaction, the master must set the cyc & stb signal of the 
        device with which it wants to communicate. Setting stb & cyc signals of appropriate 
        device depending on the selected device is handled by the interconnect 
        logic.
    */
    always @(*) begin /* COMBINATORIAL */
        case(selected_device)
            Device_RAM:     wb_ram_stb_i = wb_dbus_stb_o;
            Device_UART:    wb_uart_stb_i = wb_dbus_stb_o;

            default: begin
                wb_ram_stb_i = 1'b0;
                wb_uart_stb_i = 1'b0;
            end
        endcase
    end

    /* 
        === Ack muxing ===
        The slave used in a transaction responds by setting the ack singal. since there may be
        many slaves, depending on the selected device, the ack signal is muxed to the master.
    */
    always @(*) begin /* COMBINATORIAL */
        case(selected_device)
            Device_RAM:     wb_dbus_ack_i = wb_ram_ack_o;
            Device_UART:    wb_dbus_ack_i = wb_uart_ack_o;
            default:
                wb_dbus_ack_i = 1'b0;
        endcase
    end

endmodule