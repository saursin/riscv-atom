`include "BiDirectionalIO.v"

`default_nettype none

module GPIO
(
    // Wishbone Interface
    input   wire            wb_clk_i,
    input   wire            wb_rst_i,

    output  reg     [31:0]  wb_dat_o,

    /* verilator lint_off UNUSED */
    input   wire    [31:0]  wb_dat_i,
    /* verilator lint_on UNUSED */

    input   wire            wb_we_i,
    input   wire    [3:0]   wb_sel_i,
  
    input   wire            wb_stb_i,
    output  reg             wb_ack_o,

    inout  wire    [15:0]   gpio_io
);

localparam DIR_OUTPUT   = 1'b0;
localparam DIR_INPUT    = 1'b1;

// Set Ack_o
always @(posedge wb_clk_i) begin
  if (wb_rst_i)
    wb_ack_o <= 1'b0;
  else
    wb_ack_o <= wb_stb_i & !wb_ack_o;
end

wire    [3:0]   we  = {4{wb_we_i & wb_stb_i}} & wb_sel_i;

// Reflects current state of GPIO pins as inputs
wire    [15:0]   gpio_read_val;

// Holds GPIO pin state to output
reg     [15:0]   gpio_state = 16'h0000;

// Holds GPIO direction 
reg     [15:0]   gpio_direction = 16'h0000;


genvar i;
generate for(i=0; i<16; i=i+1) begin:BiDirIO
    BiDirectionalIO io
    (
        .dir_i  (gpio_direction[i]),
        .bit_i  (gpio_state[i]),
        .bit_o  (gpio_read_val[i]),

        .pin_io (gpio_io[i])
    );
end
endgenerate


// Handle Reads & Writes
always @(posedge wb_clk_i) begin
    if(wb_rst_i) begin
        gpio_state <= 16'h0000;
        gpio_direction <= 16'h0000;
    end
    else begin
        if (we[0]) gpio_state[7:0]      <= wb_dat_i[7:0];
        if (we[1]) gpio_state[15:8]     <= wb_dat_i[15:8];
        if (we[2]) gpio_direction[7:0]  <= wb_dat_i[23:16];
        if (we[3]) gpio_direction[15:8] <= wb_dat_i[31:24];
    end

    wb_dat_o <= {gpio_direction, gpio_read_val};
end

endmodule