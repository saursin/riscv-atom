///////////////////////////////////////////////////////////////////
//  File        : HydrogenSoC.v
//  Author      : Saurabh Singh (saurabh.s99100@gmail.com)
//  Description : HydrogenSoC is an FPGA ready SoC, it consists of
//      a single atom core with memories and communication modules.
///////////////////////////////////////////////////////////////////
`include "HydrogenSoC_Config.vh"
`include "common/Utils.vh"

`ifdef verilator
    // Macros for Verilator

`else
`ifdef SYNTHESIS_YOSYS
    // Macros for Yosys

`else
    // Macros for Xilinx ISE
    `define __ROM_INIT_FILE__ "rom.hex"
    `define __RAM_INIT_FILE__ "ram.hex"
`endif
`endif

`default_nettype none

module HydrogenSoC(
    // GLOBAL SIGNALS
    input   wire        clk_i,
    input   wire        rst_i,

    // GPIO
    inout   wire [`NGPIO-1:0] gpio_io,

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
    parameter ROM_ADR_SIZE = $clog2(`ROM_SIZE);
    parameter RAM_ADR_SIZE = $clog2(`RAM_SIZE);

    reg [31:0] ctr =32'd0;
    always @(posedge clk_i)
        ctr <= ctr + 1;


    ////////////////////////////////// Wishbone Interconnect //////////////////////////////////
    wire wb_clk_i = clk_i;
    wire wb_rst_i = rst_i;

    // ******************* FrontPort *********************
    // FrontPort Signals
    wire    [31:0]  fp_wb_adr_o     = 32'h00000000;
    wire    [31:0]  fp_wb_dat_o     = 32'h00000000;
    wire    [31:0]  fp_wb_dat_i;    `UNUSED_VAR(fp_wb_dat_i)
    wire            fp_wb_we_o      = 1'b0;
    wire    [3:0]   fp_wb_sel_o     = 4'b0000;
    wire            fp_wb_stb_o     = 1'b0;
    wire            fp_wb_ack_i;    `UNUSED_VAR(fp_wb_ack_i)
    wire            fp_wb_cyc_o     = 1'b0;

    // ********************* Core *********************
    
    // IPort Signals
    wire    [31:0]  core_iport_wb_adr_o;
    wire    [31:0]  core_iport_wb_dat_o     = 32'h000000;
    wire    [31:0]  core_iport_wb_dat_i;
    wire            core_iport_wb_we_o      = 1'b0;
    wire    [3:0]   core_iport_wb_sel_o     = 4'b1111;
    wire            core_iport_wb_stb_o;
    wire            core_iport_wb_ack_i;
    wire            core_iport_wb_cyc_o;

    
    // DPort Signals
    wire    [31:0]  core_dport_wb_adr_o;
    wire    [31:0]  core_dport_wb_dat_o;
    wire    [31:0]  core_dport_wb_dat_i;
    wire            core_dport_wb_we_o;
    wire    [3:0]   core_dport_wb_sel_o;
    wire            core_dport_wb_stb_o;
    wire            core_dport_wb_ack_i;
    wire            core_dport_wb_cyc_o;


    AtomRV_wb atom_wb_core
    (   
        .wb_clk_i       (wb_clk_i),
        .wb_rst_i       (wb_rst_i),

        .reset_vector_i  (`SOC_RESET_ADDRESS),

        // === IBUS Wishbone Master Interface ===
        .iport_wb_adr_o  (core_iport_wb_adr_o),
        .iport_wb_dat_i  (core_iport_wb_dat_i),
        .iport_wb_cyc_o  (core_iport_wb_cyc_o),
        .iport_wb_stb_o  (core_iport_wb_stb_o),
        .iport_wb_ack_i  (core_iport_wb_ack_i),
         
        // === DBUS Wishbone Master Interface ===
        .dport_wb_adr_o  (core_dport_wb_adr_o),
        .dport_wb_dat_o  (core_dport_wb_dat_o),
        .dport_wb_dat_i  (core_dport_wb_dat_i),
        .dport_wb_we_o   (core_dport_wb_we_o),
        .dport_wb_sel_o  (core_dport_wb_sel_o),
        .dport_wb_stb_o  (core_dport_wb_stb_o),
        .dport_wb_ack_i  (core_dport_wb_ack_i),
        .dport_wb_cyc_o  (core_dport_wb_cyc_o)
    );


    // ********************* Arbiter *********************
    wire    [31:0]  arb_wb_adr_o;
    reg     [31:0]  arb_wb_dat_i;
    wire    [31:0]  arb_wb_dat_o;
    wire            arb_wb_we_o;
    wire    [3:0]   arb_wb_sel_o;
    wire            arb_wb_stb_o;
    reg             arb_wb_ack_i;
    wire            arb_wb_cyc_o;

    arbiter3_wb #(
        .DATA_WIDTH (32),
        .ADDR_WIDTH (32),
        .SELECT_WIDTH (4)
    ) arb3x1 (
        .clk        (clk_i),
        .rst        (rst_i),

        // Wishbone master 0 input
        .wbm0_adr_i     (core_iport_wb_adr_o),
        .wbm0_dat_i     (core_iport_wb_dat_o),
        .wbm0_dat_o     (core_iport_wb_dat_i),
        .wbm0_we_i      (core_iport_wb_we_o),
        .wbm0_sel_i     (core_iport_wb_sel_o),
        .wbm0_stb_i     (core_iport_wb_stb_o),
        .wbm0_ack_o     (core_iport_wb_ack_i),
        .wbm0_cyc_i     (core_iport_wb_cyc_o),

        // Wishbone master 1 input
        .wbm1_adr_i     (core_dport_wb_adr_o),
        .wbm1_dat_i     (core_dport_wb_dat_o),
        .wbm1_dat_o     (core_dport_wb_dat_i),
        .wbm1_we_i      (core_dport_wb_we_o),
        .wbm1_sel_i     (core_dport_wb_sel_o),
        .wbm1_stb_i     (core_dport_wb_stb_o),
        .wbm1_ack_o     (core_dport_wb_ack_i),
        .wbm1_cyc_i     (core_dport_wb_cyc_o),

        // Wishbone master 2 input
        .wbm2_adr_i     (fp_wb_adr_o),
        .wbm2_dat_i     (fp_wb_dat_o),
        .wbm2_dat_o     (fp_wb_dat_i),
        .wbm2_we_i      (fp_wb_we_o),
        .wbm2_sel_i     (fp_wb_sel_o),
        .wbm2_stb_i     (fp_wb_stb_o),
        .wbm2_ack_o     (fp_wb_ack_i),
        .wbm2_cyc_i     (fp_wb_cyc_o),

        // Wishbone slave output
        .wbs_adr_o      (arb_wb_adr_o),
        .wbs_dat_i      (arb_wb_dat_i),
        .wbs_dat_o      (arb_wb_dat_o),
        .wbs_we_o       (arb_wb_we_o), 
        .wbs_sel_o      (arb_wb_sel_o),
        .wbs_stb_o      (arb_wb_stb_o),
        .wbs_ack_i      (arb_wb_ack_i),
        .wbs_cyc_o      (arb_wb_cyc_o)
    );


    // ********************* ROM *********************
    wire    [31:0]      rom_wb_dat_o; 
    reg                 rom_wb_stb_i;
    wire                rom_wb_ack_o;

    SinglePortRAM_wb #(
        .ADDR_WIDTH(ROM_ADR_SIZE),
        .MEM_FILE(`__ROM_INIT_FILE__)
    ) rom 
    (
        .wb_clk_i   (wb_clk_i),
        .wb_rst_i   (wb_rst_i),

        .wb_adr_i   (arb_wb_adr_o[ROM_ADR_SIZE-1:2]),
        .wb_dat_o   (rom_wb_dat_o),
        .wb_dat_i   (arb_wb_dat_o),
        .wb_we_i    (arb_wb_we_o),
        .wb_sel_i   (arb_wb_sel_o),

        .wb_stb_i   (rom_wb_stb_i),
        .wb_ack_o   (rom_wb_ack_o)
    );

    // ********************* RAM *********************

    wire    [31:0]      ram_wb_dat_o; 
    reg                 ram_wb_stb_i;
    wire                ram_wb_ack_o;

    SinglePortRAM_wb #(
        .ADDR_WIDTH(RAM_ADR_SIZE),
        .MEM_FILE(`__RAM_INIT_FILE__)
    ) ram 
    (
        .wb_clk_i   (wb_clk_i),
        .wb_rst_i   (wb_rst_i),

        .wb_adr_i   (arb_wb_adr_o[RAM_ADR_SIZE-1:2]),
        .wb_dat_o   (ram_wb_dat_o),
        .wb_dat_i   (arb_wb_dat_o),
        .wb_we_i    (arb_wb_we_o),
        .wb_sel_i   (arb_wb_sel_o),

        .wb_stb_i   (ram_wb_stb_i),
        .wb_ack_o   (ram_wb_ack_o)
    );


    //////////////////////////////////////////////////
    // UART
    wire    [31:0]  uart_wb_dat_o;
    reg             uart_wb_stb_i;
    wire            uart_wb_ack_o;
    
    UART uart (
        .wb_clk_i   (wb_clk_i),
        .wb_rst_i   (wb_rst_i),

        .wb_adr_i   (arb_wb_adr_o[3:2]),
        .wb_dat_o   (uart_wb_dat_o),
        .wb_dat_i   (arb_wb_dat_o),
        .wb_we_i    (arb_wb_we_o),
        .wb_sel_i   (arb_wb_sel_o),

        .wb_stb_i   (uart_wb_stb_i),
        .wb_ack_o   (uart_wb_ack_o),

        .rx_i       (uart_rx),
        .tx_o       (uart_tx)
    );
    

    ////////////////////////////////////////////////////
    // GPIO
    wire    [31:0]  gpio_wb_dat_o;
    reg             gpio_wb_stb_i;
    wire            gpio_wb_ack_o;
    
    GPIO #(
        .N(`NGPIO)
    ) gpio (
        .wb_clk_i   (wb_clk_i),
        .wb_rst_i   (wb_rst_i),

        .wb_adr_i   (arb_wb_adr_o[3:2]),
        .wb_dat_o   (gpio_wb_dat_o),
        .wb_dat_i   (arb_wb_dat_o),
        .wb_we_i    (arb_wb_we_o),
        .wb_sel_i   (arb_wb_sel_o),
    
        .wb_stb_i   (gpio_wb_stb_i),
        .wb_ack_o   (gpio_wb_ack_o),

        .gpio_io    (gpio_io)
    );

    ////////////////////////////////////////////////////
    // Wishbone Interconnect Logic

    // Devices
    localparam Device_None      = 4'd0;
    localparam Device_ROM       = 4'd1;
    localparam Device_RAM       = 4'd2;
    localparam Device_UART      = 4'd3;
    localparam Device_GPIO      = 4'd4;

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
        
        if(arb_wb_cyc_o) begin
            /* verilator lint_off UNSIGNED */
            if(arb_wb_adr_o >= `ROM_ADDR && arb_wb_adr_o < `ROM_ADDR+`ROM_SIZE)
                selected_device = Device_ROM;
            /* verilator lint_on UNSIGNED */

            else if(arb_wb_adr_o >= `RAM_ADDR && arb_wb_adr_o < `RAM_ADDR+`RAM_SIZE)
                selected_device = Device_RAM;
            
            else if (arb_wb_adr_o >= `UART_ADDR && arb_wb_adr_o < `UART_ADDR+`UART_SIZE)
                selected_device = Device_UART;

            else if (arb_wb_adr_o >= `GPIO_ADDR && arb_wb_adr_o < `GPIO_ADDR+`GPIO_SIZE)
                selected_device = Device_GPIO;

            else begin
                selected_device = Device_None;

                `ifdef verilator
                    $display("RTL-ERROR: Unknown Device Selected: 0x%x\nHaulting simulation...", arb_wb_adr_o);
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
            Device_ROM:         arb_wb_dat_i = rom_wb_dat_o;
            Device_RAM:         arb_wb_dat_i = ram_wb_dat_o;
            Device_UART:        arb_wb_dat_i = uart_wb_dat_o;
            Device_GPIO:        arb_wb_dat_i = gpio_wb_dat_o;

            default: begin
                arb_wb_dat_i = 32'h00000000;
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
          rom_wb_stb_i      = 1'b0;
          ram_wb_stb_i      = 1'b0;
          uart_wb_stb_i     = 1'b0;
          gpio_wb_stb_i     = 1'b0;
                     
        case(selected_device)
            Device_ROM:         rom_wb_stb_i        = arb_wb_stb_o;
            Device_RAM:         ram_wb_stb_i        = arb_wb_stb_o;
            Device_UART:        uart_wb_stb_i       = arb_wb_stb_o;
            Device_GPIO:        gpio_wb_stb_i       = arb_wb_stb_o;

            default: begin
                rom_wb_stb_i        = 1'b0;
                ram_wb_stb_i        = 1'b0;
                uart_wb_stb_i       = 1'b0;
                gpio_wb_stb_i       = 1'b0;
                gpio_wb_stb_i       = 1'b0;
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
            Device_ROM:         arb_wb_ack_i = rom_wb_ack_o;
            Device_RAM:         arb_wb_ack_i = ram_wb_ack_o;
            Device_UART:        arb_wb_ack_i = uart_wb_ack_o;
            Device_GPIO:        arb_wb_ack_i = gpio_wb_ack_o;
            default:
                arb_wb_ack_i = 1'b0;
        endcase
    end

endmodule
