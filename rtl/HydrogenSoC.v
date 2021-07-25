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
    input rst_i,

    output wire [31:0] addr
);
    wire wb_clk_i = clk_i;
    wire wb_rst_i = rst_i;

    assign addr = wb_ibus_adr_o | wb_dbus_adr_o;

    //////////////////////////////////////////
    // Global Bus Signals

    wire    [31:0]  wb_ibus_adr_o;
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
    .ADDR_WIDTH(16),
    .MEM_INIT_FILE("/home/frozenalpha/git/riscv/riscv-atom/build/init/code.hex")
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
    // 8kb
    wire    [31:0] wb_ram_data_o; 
    reg            wb_ram_stb_i;
    reg            wb_ram_ack_o;

    SinglePortRAM_wb #(
    .ADDR_WIDTH(13),
    .MEM_FILE("/home/frozenalpha/git/riscv/riscv-atom/build/init/data.hex")
    ) dmem 
    (
    // Wishbone Interface
        .wb_clk_i   (wb_clk_i),
        .wb_rst_i   (wb_rst_i),

        .wb_adr_i   (wb_dbus_adr_o[12:2]),
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

    // Device selection
    reg [2:0] selected_device /* verilator public */;
    always @(*) begin /* COMBINATORIAL */
        if(wb_dbus_cyc_o) begin
            if(wb_dbus_adr_o >= 32'h04000000 && wb_dbus_adr_o < 32'h08000000)
                selected_device = Device_RAM;
            else if (wb_dbus_adr_o == 32'h08000000) // byte addresses 8000000 to 08000003
                selected_device = Device_UART;
            else
                $display("Unknown Device Selected: 0x%x", wb_dbus_adr_o);
        end
        else begin
            selected_device = Device_None;
        end
    end

    // Data Muxing
    always @(*) begin /* COMBINATORIAL */
        case(selected_device)
            Device_RAM:     wb_dbus_dat_i = wb_ram_data_o;
            Device_UART:    wb_dbus_dat_i = wb_uart_data_o;

            default: begin
                wb_dbus_dat_i = 32'h00000000;
            end
        endcase
    end

    // Stb muxing
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

    // Ack muxing
    always @(*) begin /* COMBINATORIAL */
        case(selected_device)
            Device_RAM:     wb_dbus_ack_i = wb_ram_ack_o;
            Device_UART:    wb_dbus_ack_i = wb_uart_ack_o;
            default:
                wb_dbus_ack_i = 1'b0;
        endcase
    end

endmodule