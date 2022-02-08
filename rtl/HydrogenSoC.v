///////////////////////////////////////////////////////////////////
//  File        : HydrogenSoC.v
//  Author      : Saurabh Singh (saurabh.s99100@gmail.com)
//  Description : HydrogenSoC is an FPGA ready SoC, it consists of
//      a single atom core with memories and communication modules.
///////////////////////////////////////////////////////////////////

`include "Timescale.vh"
`include "HydrogenSoC_Config.vh"

`include "core/AtomRV_wb.v"
`include "uncore/DualPortRAM_wb.v"
//`include "uncore/SinglePortROM_wb.v"
`include "uncore/SinglePortRAM_wb.v"
`include "uncore/simpleuart_wb.v"
`include "uncore/GPIO.v"

`ifndef verilator
    // Defaults for Xilinx ISE
    `define __IMEM_INIT_FILE__ "code.hex"
    `define __DMEM_INIT_FILE__ "data.hex"
`endif

`default_nettype none

module HydrogenSoC
(
    // GLOBAL SIGNALS
    input   wire        clk_i,
    input   wire        rst_i,
    
    // GPIO
    inout   wire [31:0] gpio_io,

    // UART
    input   wire        uart_usb_rx_i,
    output  wire        uart_usb_tx_o,

    input   wire        uart_io_rx_i,
    output  wire        uart_io_tx_o,
     
    // UART MUX
    input   wire        uart_mux_sel,
     
    // TEST POINTS
    output  wire        uart_rx_test_point_o,
    output  wire        uart_tx_test_point_o
);
    ////////////////////////////////////////
    // UART MUX
    wire uart_rx = uart_mux_sel ? uart_io_rx_i : uart_usb_rx_i;
    
    wire uart_tx;
    assign uart_io_tx_o = uart_mux_sel ? uart_tx : 1'b1;
    assign uart_usb_tx_o = uart_mux_sel ? 1'b1 : uart_tx;
    
    // TEST POINTS
    assign uart_rx_test_point_o = uart_rx;
    assign uart_tx_test_point_o = uart_tx;
    
    
    //////////////////////////////////////////
    // SoC Parameters

    parameter IMEM_ADR_SIZE = 15;   // 32KB
    parameter DMEM_ADR_SIZE = 13;   // 8KB

    //////////////////////////////////////////
    // Global Bus Signals

    wire wb_clk_i = clk_i;
    wire wb_rst_i = rst_i;

    wire    [31:0]  wb_ibus_adr_o;
    `UNUSED_VAR(wb_ibus_adr_o)

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
    wire            wb_dbus_cyc_o   /* verilator public */;
    


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

    // SinglePortROM_wb #(
    //     .ADDR_WIDTH(IMEM_ADR_SIZE),
    //     .MEM_INIT_FILE(`__IMEM_INIT_FILE__)
    // ) imem
    // (
    //     .clk_i      (wb_clk_i),
    //     .rst_i      (wb_rst_i),

    //     .addr_i     (wb_ibus_adr_o[IMEM_ADR_SIZE-1:2]),
    //     .data_o     (wb_ibus_dat_i),

    //     .stb_i      (wb_ibus_stb_o),
    //     .ack_o      (wb_ibus_ack_i)
    // );

    wire    [31:0] wb_iram_data_o;
    reg            wb_iram_stb_i;
    wire           wb_iram_ack_o;


    DualPortRAM_wb #(
        .ADDR_WIDTH(IMEM_ADR_SIZE),
        .MEM_FILE(`__IMEM_INIT_FILE__)
    ) imem
    (
        // Global Signals^M
        .wb_clk_i           (wb_clk_i),
        .wb_rst_i           (wb_rst_i),

        .wb_adr_i           (wb_dbus_adr_o[IMEM_ADR_SIZE-1:2]),
        .wb_dat_o           (wb_iram_data_o),
        .wb_dat_i           (wb_dbus_dat_o),
        .wb_we_i            (wb_dbus_we_o),
        .wb_sel_i           (wb_dbus_sel_o),

        .wb_stb_i           (wb_iram_stb_i),
        .wb_ack_o           (wb_iram_ack_o),

        .wb_roport_adr_i    (wb_ibus_adr_o[IMEM_ADR_SIZE-1:2]),
        .wb_roport_dat_o    (wb_ibus_dat_i),
        .wb_roport_stb_i    (wb_ibus_stb_o),
        .wb_roport_ack_o    (wb_ibus_ack_i)
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
    reg             wb_uart_stb_i;
    wire            wb_uart_ack_o;
    
    simpleuart_wb uart (
        .wb_clk_i   (wb_clk_i),
        .wb_rst_i   (wb_rst_i),

        .wb_adr_i   (wb_dbus_adr_o[2]),
        .wb_dat_o   (wb_uart_data_o),
        .wb_dat_i   (wb_dbus_dat_o),
        .wb_we_i    (wb_dbus_we_o),
        .wb_sel_i   (wb_dbus_sel_o),

        .wb_stb_i   (wb_uart_stb_i),
        .wb_ack_o   (wb_uart_ack_o),

        .rx_i       (uart_rx),
        .tx_o       (uart_tx)
    );
    

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

        .gpio_io     (gpio_io[15:0])
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

        .gpio_io     (gpio_io[31:16])
    );

    ////////////////////////////////////////////////////
    // Wishbone Interconnect Logic

    // Devices
    localparam Device_None      = 4'd0;
    localparam Device_RAM       = 4'd1;
    localparam Device_IRAM      = 4'd2;
    localparam Device_UART      = 4'd3;
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
          // default 
          //selected_device = Device_None;
        
        if(wb_dbus_cyc_o) begin
            /* verilator lint_off UNSIGNED */
            if(wb_dbus_adr_o >= `IRAM_ADDR && wb_dbus_adr_o < `IRAM_ADDR+`IRAM_SIZE)
                selected_device = Device_IRAM;
            /* verilator lint_on UNSIGNED */

            else if(wb_dbus_adr_o >= `RAM_ADDR && wb_dbus_adr_o < `RAM_ADDR+`RAM_SIZE)
                selected_device = Device_RAM;
            
            else if (wb_dbus_adr_o >= `UART_ADDR && wb_dbus_adr_o < `UART_ADDR+`UART_SIZE)
                selected_device = Device_UART;

            else if (wb_dbus_adr_o >= `GPIO0_ADDR && wb_dbus_adr_o < `GPIO0_ADDR+`GPIO0_SIZE)
                selected_device = Device_GPIO0;

            else if (wb_dbus_adr_o >= `GPIO1_ADDR && wb_dbus_adr_o < `GPIO1_ADDR+`GPIO1_SIZE)
                selected_device = Device_GPIO1;

            else begin
                selected_device = Device_None;

                `ifdef verilator
                    $display("RTL-ERROR: Unknown Device Selected: 0x%x\nHaulting simulation...", wb_dbus_adr_o);
                    $finish();
                `endif
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
            Device_IRAM:        wb_dbus_dat_i = wb_iram_data_o;
            Device_UART:        wb_dbus_dat_i = wb_uart_data_o;
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
          // Defaults
          wb_ram_stb_i        = 1'b0;
          wb_iram_stb_i       = 1'b0;
          wb_uart_stb_i       = 1'b0;
          wb_gpio0_stb_i      = 1'b0;
          wb_gpio1_stb_i      = 1'b0;
                     
        case(selected_device)
            Device_RAM:         wb_ram_stb_i        = wb_dbus_stb_o;
            Device_IRAM:        wb_iram_stb_i       = wb_dbus_stb_o;
            Device_UART:        wb_uart_stb_i       = wb_dbus_stb_o;
            Device_GPIO0:       wb_gpio0_stb_i      = wb_dbus_stb_o;
            Device_GPIO1:       wb_gpio1_stb_i      = wb_dbus_stb_o;

            default: begin
                wb_ram_stb_i        = 1'b0;
                wb_iram_stb_i       = 1'b0;
                wb_uart_stb_i       = 1'b0;
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
            Device_IRAM:        wb_dbus_ack_i = wb_iram_ack_o;
            Device_UART:        wb_dbus_ack_i = wb_uart_ack_o;
            Device_GPIO0:       wb_dbus_ack_i = wb_gpio0_ack_o;
            Device_GPIO1:       wb_dbus_ack_i = wb_gpio1_ack_o;
            default:
                wb_dbus_ack_i = 1'b0;
        endcase
    end

endmodule
