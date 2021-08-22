`default_nettype none

`include "Timescale.vh"
`include "Config.vh"
`include "core/AtomRV_wb.v"
`include "uncore/SinglePortROM_wb.v"
`include "uncore/SinglePortRAM_wb.v"
//#`include "uncore/DummyUART.v"
`include "uncore/simpleuart.v"
`include "uncore/GPIO.v"

`ifndef verilator
// Defaults for Xilinx ISE
`define __IMEM_INIT_FILE__ "blinky_code.hex"
`define __DMEM_INIT_FILE__ "blinky_data.hex"
`endif

/**
 *  === Hydrogen SoC ===
 *  Barebone SoC housing a single atom core, instruction memory, data memory and a uart slave.
 */

module HydrogenSoC
(
    input wire clk_i,
    input wire rst_i,

    output wire [7:0] gpio_o,

    input   wire        uart_rx_i,
    output  wire        uart_tx_o
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
        .MEM_INIT_FILE(`__IMEM_INIT_FILE__)
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
    wire           wb_ram_ack_o;

    SinglePortRAM_wb #(
        .ADDR_WIDTH(DMEM_ADR_SIZE),
        .MEM_FILE(`__DMEM_INIT_FILE__)
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
    wire    [31:0]  wb_uart_data_o;
    wire    [31:0]  wb_uart_div_o;
    
    reg             wb_uart_div_stb_i;
    reg             wb_uart_data_stb_i;

    reg             wb_uart_div_ack_o;
    reg             wb_uart_data_ack_o;
    /* verilator lint_off UNUSED */
    wire            uart_wait;
    /* verilator lint_on UNUSED */

	simpleuart simpleuart (
		.clk         (wb_clk_i),
		.resetn      (wb_rst_i),

		.ser_tx      (uart_tx_o),
		.ser_rx      (uart_rx_i),

		.reg_div_we  (wb_uart_div_stb_i ? wb_dbus_sel_o : 4'b 0000),
		.reg_div_di  (wb_dbus_dat_o),
		.reg_div_do  (wb_uart_div_o),

		.reg_dat_we  (wb_uart_data_stb_i ? wb_dbus_sel_o[0] : 1'b 0),
		.reg_dat_re  (wb_uart_data_stb_i && !wb_dbus_stb_o),
		.reg_dat_di  (wb_dbus_dat_o[7:0]),
		.reg_dat_do  (wb_uart_data_o),
		.reg_dat_wait(uart_wait)
	);

    // Set Ack_o for Div
    always @(posedge wb_clk_i) begin
    if (wb_rst_i)
        wb_uart_div_ack_o <= 1'b0;
    else
        wb_uart_div_ack_o <= wb_uart_div_stb_i & !wb_uart_div_ack_o;
    end

    // Set Ack_o for Data
    always @(posedge wb_clk_i) begin
    if (wb_rst_i)
        wb_uart_data_ack_o <= 1'b0;
    else
        wb_uart_data_ack_o <= wb_uart_data_stb_i & !wb_uart_data_ack_o;
    end


    /*DummyUART uart
    (
        .wb_clk_i   (wb_clk_i),
        .wb_rst_i   (wb_rst_i),
        
        .wb_dat_o   (wb_uart_data_o),
        .wb_dat_i   (wb_dbus_dat_o),
        .wb_we_i    (wb_dbus_we_o),
        .wb_sel_i   (wb_dbus_sel_o),

        .wb_stb_i   (wb_uart_stb_i),
        .wb_ack_o   (wb_uart_ack_o)
    );*/

    ////////////////////////////////////////////////////
    // GPIO
    wire    [31:0] wb_gpio0_data_o;
    reg     wb_gpio0_stb_i;
    wire    wb_gpio0_ack_o;
    
    GPIO gpio0
    (
        .wb_clk_i   (wb_clk_i),
        .wb_rst_i   (wb_rst_i),

        .wb_dat_o   (wb_gpio0_data_o),
        .wb_dat_i   (wb_dbus_dat_o),
        .wb_we_i    (wb_dbus_we_o),
        .wb_sel_i   (wb_dbus_sel_o),
    
        .wb_stb_i   (wb_gpio0_stb_i),
        .wb_ack_o   (wb_gpio0_ack_o),

        .gpio_o     (gpio_o[3:0])
    );

    wire    [31:0] wb_gpio1_data_o;
    reg     wb_gpio1_stb_i;
    wire    wb_gpio1_ack_o;
    
    GPIO gpio1
    (
        .wb_clk_i   (wb_clk_i),
        .wb_rst_i   (wb_rst_i),

        .wb_dat_o   (wb_gpio1_data_o),
        .wb_dat_i   (wb_dbus_dat_o),
        .wb_we_i    (wb_dbus_we_o),
        .wb_sel_i   (wb_dbus_sel_o),
    
        .wb_stb_i   (wb_gpio1_stb_i),
        .wb_ack_o   (wb_gpio1_ack_o),

        .gpio_o     (gpio_o[7:4])
    );

    ////////////////////////////////////////////////////
    // Wishbone Interconnect Logic

    // Devices
    localparam Device_None      = 4'd0;
    localparam Device_RAM       = 4'd1;
    localparam Device_UART_Div  = 4'd2;
    localparam Device_UART_Data = 4'd3;
    localparam Device_GPIO0     = 4'd4;
    localparam Device_GPIO1     = 4'd5;

    /*
        === Device selection ===
        A device is selected among available devices, depending on which region of memory 
        does the cpu wants to access. If none of the device is selected (i.e. cpu tries to 
        access a region of memory which is not mapped to any device) an errror is thrown &
        simulation is haulted.
    */
    reg [3:0] selected_device /* verilator public */;
    always @(*) begin /* COMBINATORIAL */
        if(wb_dbus_cyc_o) begin
            if(wb_dbus_adr_o >= 32'h04000000 && wb_dbus_adr_o < 32'h08000000)
                selected_device = Device_RAM;
            else if (wb_dbus_adr_o == 32'h08000000) // byte addresses 8000000 to 08000003
                selected_device = Device_UART_Div;
            else if (wb_dbus_adr_o == 32'h08000004) // byte addresses 8000004
                selected_device = Device_UART_Data;

            else if (wb_dbus_adr_o == 32'h08000100) // byte addresses 8000010 to 0800001f
                selected_device = Device_GPIO0;
            else if (wb_dbus_adr_o == 32'h08000104) // byte addresses 8000010 to 0800001f
                selected_device = Device_GPIO1;

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
            Device_RAM:         wb_dbus_dat_i = wb_ram_data_o;
            Device_UART_Div:    wb_dbus_dat_i = wb_uart_div_o;
            Device_UART_Data:   wb_dbus_dat_i = wb_uart_data_o;
            Device_GPIO0:       wb_dbus_dat_i = wb_gpio0_data_o;
            Device_GPIO1:       wb_dbus_dat_i = wb_gpio1_data_o;

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
            Device_RAM:         wb_ram_stb_i        = wb_dbus_stb_o;
            Device_UART_Div:    wb_uart_div_stb_i   = wb_dbus_stb_o;
            Device_UART_Data:   wb_uart_data_stb_i  = wb_dbus_stb_o;
            Device_GPIO0:       wb_gpio0_stb_i      = wb_dbus_stb_o;
            Device_GPIO1:       wb_gpio1_stb_i      = wb_dbus_stb_o;

            default: begin
                wb_ram_stb_i        = 1'b0;
                wb_uart_div_stb_i   = 1'b0;
                wb_uart_data_stb_i  = 1'b0;
                wb_gpio0_stb_i      = 1'b0;
                wb_gpio1_stb_i      = 1'b0;
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
            Device_RAM:         wb_dbus_ack_i = wb_ram_ack_o;
            Device_UART_Div:    wb_dbus_ack_i = wb_uart_div_ack_o;
            Device_UART_Data:   wb_dbus_ack_i = wb_uart_data_ack_o;
            Device_GPIO0:       wb_dbus_ack_i = wb_gpio0_ack_o;
            Device_GPIO1:       wb_dbus_ack_i = wb_gpio1_ack_o;
            default:
                wb_dbus_ack_i = 1'b0;
        endcase
    end

endmodule
