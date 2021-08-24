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

    output  wire    [3:0]   gpio_o
);


// Set Ack_o
always @(posedge wb_clk_i) begin
  if (wb_rst_i)
    wb_ack_o <= 1'b0;
  else
    wb_ack_o <= wb_stb_i & !wb_ack_o;
end

wire    [3:0]   we  = {4{wb_we_i & wb_stb_i}} & wb_sel_i;

// Holds GPIO state
reg     [3:0]   gpio_state = 4'b0000;

assign gpio_o = gpio_state;

// Handle Reads & Writes
always @(posedge wb_clk_i) begin
    if(wb_rst_i)
        gpio_state <= 4'b0000;
    else begin
        if (we[0]) gpio_state[0]   <= wb_dat_i[0];
        if (we[1]) gpio_state[1]   <= wb_dat_i[8];
        if (we[2]) gpio_state[2]   <= wb_dat_i[16];
        if (we[3]) gpio_state[3]   <= wb_dat_i[24];
    end

    wb_dat_o <= {7'b0000000, gpio_state[3], 7'b0000000, gpio_state[2], 7'b0000000, gpio_state[1], 7'b0000000, gpio_state[0]};
end

endmodule