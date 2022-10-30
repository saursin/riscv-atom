////////////////////////////////////////////////////////////////////   
//  File        : simpleuart_wb.v
//  Author      : Saurabh Singh (saurabh.s99100@gmail.com)
//  Description : Wishbone wrapper for simpleuart.v by clifford wolf
////////////////////////////////////////////////////////////////////   

`default_nettype none

module UART#(
    parameter DEFAULT_DIV = 3
)(
    // Wishbone Interface
    input   wire                    wb_clk_i,
    input   wire                    wb_rst_i,

    input   wire  [3:2]        		wb_adr_i,
    output  reg   [31:0] 	        wb_dat_o,
    input   wire  [31:0] 	       	wb_dat_i /* verilator public */,
    input   wire 		            wb_we_i,
    input   wire  [3:0] 	        wb_sel_i,

    input   wire                    wb_stb_i,
    output  reg 		            wb_ack_o,

    // Serial Interface
    /* verilator lint_off UNUSED */
    input   wire                    rx_i,
    /* verilator lint_on UNUSED */

    /* verilator lint_off UNDRIVEN */
    output  wire                    tx_o
    /* verilator lint_on UNDRIVEN */
);
////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    if __ATOMSIM_SIMULATION__ is defined, this module behaves as a stub. simpleuart is not 
    instantiated, instead reg_status, reg_data, and reg_div are converted to actual registers
    which are written externally by atomsim for the purpose of the simulation of uart communication.
*/

// Set Ack_o
always @(posedge wb_clk_i) begin
if (wb_rst_i)
    wb_ack_o <= 1'b0;
else
    wb_ack_o <= wb_stb_i & !wb_ack_o;
end

wire    [3:0]   we  = {4{wb_we_i & wb_stb_i}} & wb_sel_i;

// registers
reg     [7:0]   reg_lcr = 0;
reg     [31:0]  reg_div = 0;

wire reg_data_we = (wb_adr_i == 2'b11) && wb_we_i && wb_stb_i && wb_sel_i[0];
wire reg_data_re = (wb_adr_i == 2'b11) && !wb_we_i && wb_stb_i;

// Handle Writes
always @(posedge wb_clk_i) begin
    if(wb_rst_i) begin
        reg_lcr <= 0;
        reg_div <= DEFAULT_DIV;
    end else begin
        case(wb_adr_i)
            2'b01: /* LCR */ begin
                if(we[0]) reg_lcr <= wb_dat_i[7:0];
            end           
            2'b11: /* DIV */ begin
                if (we[0])  reg_div[7:0]      <= wb_dat_i[7:0];
                if (we[1])  reg_div[15:8]     <= wb_dat_i[15:8];
                if (we[2])  reg_div[23:16]    <= wb_dat_i[23:16];
                if (we[3])  reg_div[31:24]    <= wb_dat_i[31:24];
            end
            default: begin end
        endcase
    end
end

// Handle Reads
always @(*) /* COMBINATORIAL */ begin
    case(wb_adr_i)
        2'b00: /* THR-RBR */    wb_dat_o = {24'd0, core_reg_data};
        2'b01: /* LCR */        wb_dat_o = {24'd0, reg_lcr};
        2'b10: /* LSR */        wb_dat_o = {27'd0, core_rx_parity_bit, core_err_parity, core_err_framing, core_send_buf_empty, core_recv_buf_valid};
        2'b11: /* DIV */        wb_dat_o = reg_div;
    endcase
end

wire    [7:0]   core_reg_data;
wire            core_send_buf_empty;
wire            core_recv_buf_valid;
wire            core_err_framing;
wire            core_err_parity;
wire            core_rx_parity_bit;        

// Core Instance
UART_core uart_core_i
(
    .clk                    (wb_clk_i),
    .rst                    (wb_rst_i),
    .ser_tx                 (tx_o),
    .ser_rx                 (reg_lcr[7] ? tx_o : rx_i), // loopback
    .divisor                (reg_div),
    .reg_dat_we             (reg_data_we),
    .reg_dat_re             (reg_data_re),
    .reg_dat_di             (wb_dat_i[7:0]),
    .reg_dat_do             (core_reg_data),
    
    .rx_en                  (reg_lcr[0]),
    .tx_en                  (reg_lcr[1]),
    .enable_dual_stop_bit   (reg_lcr[2]),
    .enable_parity          (reg_lcr[3]),
    .even_parity            (reg_lcr[4]),


    .tx_buf_empty           (core_send_buf_empty),
    .rx_buf_valid           (core_recv_buf_valid),
    .err_framing            (core_err_framing),
    .err_parity             (core_err_parity),
    .rx_parity              (core_rx_parity_bit)
);


endmodule
