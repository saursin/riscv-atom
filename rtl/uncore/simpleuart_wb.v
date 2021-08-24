`default_nettype none

`include "simpleuart.v"

module simpleuart_wb
(
    // Wishbone Interface
    input   wire                    wb_clk_i,
    input   wire                    wb_rst_i,

    input   wire  [2:2]        		wb_adr_i,
    output  wire  [31:0] 	        	wb_dat_o,
    input   wire  [31:0] 	       	wb_dat_i,
    input   wire 		            	wb_we_i,
    input   wire  [3:0] 	        	wb_sel_i,

    input   wire                    wb_stb_i,
    output  reg 		            	wb_ack_o,

    // Serial Interface
    input   wire                    rx_i,
    output  wire                    tx_o
);
////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    Register Description
    ---------------------

    let base address = 0x00000000

    0x00000000 : 8-bit data register (D-Reg)                            [ Read & Write ]
        To transmit a byte      : Write to this register
        To get recieved value   : Read from this register.
            if no value is recieved or value is already read then reg will contain -1.
        
    0x00000001 : 8-bit status        (S-Reg)                            [ Read Only ]
        bit[0] = recv_buf_valid :   recieve buffer has a valid value
        bit[1] = tx_busy        :   Transmitter is busy transmitting a byte  

    0x00000004 : 32-bit Clock Divider Register (CD-Reg)                 [ Read & Write ]
        CD register sets the speed of uart tx & rx.
        if your clk freq is Fc and target baud frequency is Fb then formula for correct value 
        of CD reg is:
            value = (Fc / Fb) - 2;
*/

// register values
wire [7:0]  reg_status;
wire [7:0]  reg_data;
wire [31:0] reg_div;

// Register selection
localparam SDReg = 1'b0;        // Status and Data register
localparam CDReg = 1'b1;        // Clock Divider register

wire select_sd_reg = (wb_adr_i == SDReg);
wire select_cd_reg = !select_sd_reg;

// Output data bus
assign  wb_dat_o    = select_cd_reg ? reg_div : {16'd0, reg_status, reg_data};

// Write enables
wire    [3:0]   reg_div_we  = select_cd_reg ? {4{wb_we_i & wb_stb_i}} & wb_sel_i : 4'b0000;
wire            reg_data_we = select_sd_reg ? wb_we_i & wb_stb_i & wb_sel_i[0] : 1'b0;

// Read enables
reg reg_data_re = 0;
always @(posedge wb_clk_i)
    reg_data_re <= select_sd_reg ? !wb_we_i & wb_stb_i  & wb_sel_i[0] : 1'b0;

simpleuart#(
    .DEFAULT_DIV(1)
) 
simpleuart (
    .clk         (wb_clk_i),
    .reset       (wb_rst_i),

    .ser_tx      (tx_o),
    .ser_rx      (rx_i),

    .reg_div_we  (reg_div_we),
    .reg_div_di  (wb_dat_i),
    .reg_div_do  (reg_div),

    .reg_dat_we  (reg_data_we),
    .reg_dat_re  (reg_data_re),
    .reg_dat_di  (wb_dat_i[7:0]),
    .reg_dat_do  (reg_data),
    .reg_status  (reg_status)
);

// Set Ack_o
always @(posedge wb_clk_i) begin
if (wb_rst_i)
    wb_ack_o <= 1'b0;
else
    wb_ack_o <= wb_stb_i & !wb_ack_o;
end

endmodule