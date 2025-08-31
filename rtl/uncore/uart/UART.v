////////////////////////////////////////////////////////////////////   
//  File        : simpleuart_wb.v
//  Author      : Saurabh Singh (saurabh.s99100@gmail.com)
//  Description : Wishbone wrapper for simpleuart.v by clifford wolf
////////////////////////////////////////////////////////////////////   

`default_nettype none

module UART#(
    parameter DEFAULT_DIV = 1,
    parameter FIFO_EN = 1,
    parameter FIFO_DEPTH = 4
)(
    // Wishbone Interface
    input   wire                    wb_clk_i,
    input   wire                    wb_rst_i,

    input   wire  [3:2]        		wb_adr_i,
    output  reg   [31:0] 	        wb_dat_o,
    input   wire  [31:0] 	       	wb_dat_i,
    input   wire 		            wb_we_i,
    input   wire  [3:0] 	        wb_sel_i,

    input   wire                    wb_stb_i,
    output  reg 		            wb_ack_o,

    // Serial Interface
    input   wire                    rx_i,
    output  wire                    tx_o
);
////////////////////////////////////////////////////////////////////////////////////////////////////

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

wire reg_data_we = (wb_adr_i == 2'b00) && wb_we_i && wb_stb_i && wb_sel_i[0];
wire reg_data_re = (wb_adr_i == 2'b00) && !wb_we_i && wb_stb_i;

wire rxfifo_empty_o;
wire rxfifo_full_o;
wire [7:0] rxfifo_data_o;

wire txfifo_full_o;
wire txfifo_empty_o;
wire [7:0] txfifo_data_o;
generate
    if (FIFO_EN) begin: fifo
        FIFO_sync #(
            .DEPTH(FIFO_DEPTH),
            .DATAW(8)
        ) rxfifo (
            .clk_i      (wb_clk_i),
            .rst_i      (wb_rst_i),
            .we_i       (core_recv_buf_valid & reg_lcr[0]),
            .re_i       (reg_data_re & wb_ack_o),
            .dat_i      (core_reg_data),
            .dat_o      (rxfifo_data_o),
            .full_o     (rxfifo_full_o),
            .empty_o    (rxfifo_empty_o)
        );

        FIFO_sync #(
            .DEPTH(FIFO_DEPTH),
            .DATAW(8)
        ) txfifo (
            .clk_i      (wb_clk_i),
            .rst_i      (wb_rst_i),
            .we_i       (reg_data_we & wb_ack_o),
            .re_i       (core_send_buf_empty & reg_lcr[1]),
            .dat_i      (wb_dat_i[7:0]),
            .dat_o      (txfifo_data_o),
            .full_o     (txfifo_full_o),
            .empty_o    (txfifo_empty_o)
        );
    end else begin: no_fifo
        assign rxfifo_empty_o = 1'dx;
        assign rxfifo_full_o = 1'dx;
        assign rxfifo_data_o = 8'dx;
        assign txfifo_full_o = 1'dx;
        assign txfifo_empty_o = 1'dx;
        assign txfifo_data_o = 8'dx;
    end
endgenerate

// Handle Writes
always @(posedge wb_clk_i) begin
    if(wb_rst_i) begin
        reg_lcr <= 'b11;
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
        2'b00: /* THR-RBR */    wb_dat_o = {24'd0, FIFO_EN ? rxfifo_data_o: core_reg_data};
        2'b01: /* LCR */        wb_dat_o = {24'd0, reg_lcr};
        2'b10: /* LSR */        wb_dat_o = {27'd0, core_rx_parity_bit, core_err_parity, core_err_framing, 
                                                (FIFO_EN ? ~txfifo_full_o : core_send_buf_empty), 
                                                (FIFO_EN ? ~rxfifo_empty_o: core_recv_buf_valid)
                                            };
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
    .reg_dat_we             (FIFO_EN ? ~txfifo_empty_o : reg_data_we),
    .reg_dat_re             (FIFO_EN ? ~rxfifo_full_o : reg_data_re),
    .reg_dat_di             (FIFO_EN ? txfifo_data_o : wb_dat_i[7:0]),
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
