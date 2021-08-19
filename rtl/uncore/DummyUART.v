`default_nettype none

module DummyUART
(
  // Wishbone Interface
  input   wire                wb_clk_i,
  input   wire 		            wb_rst_i,

  output  wire  [31:0] 	      wb_dat_o,
  input   wire  [31:0] 	      wb_dat_i,
  input   wire 		            wb_we_i,
  input   wire  [3:0] 	      wb_sel_i,

  input   wire                wb_stb_i,
  output  reg 		            wb_ack_o
);


reg     [31:0]  outWord /* verilator public */ = 32'h00000000;
wire    [31:0]  inWord  /* verilator public */ = wb_dat_i;
wire    [3:0] 	writeEn /* verilator public */ = {4{wb_we_i & wb_stb_i}} & wb_sel_i;


assign wb_dat_o = outWord;

// Set Ack_o
always @(posedge wb_clk_i) begin
  if (wb_rst_i)
    wb_ack_o <= 1'b0;
  else
    wb_ack_o <= wb_stb_i & !wb_ack_o;
end

endmodule