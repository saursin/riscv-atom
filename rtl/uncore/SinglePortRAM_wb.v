`default_nettype none

module SinglePortRAM_wb #(
  // Parameters
  parameter ADDR_WIDTH = 16,
  parameter MEM_FILE = ""
)
(
  // Wishbone Interface
  input   wire                    wb_clk_i,
  input   wire 		                wb_rst_i,

  input   wire  [ADDR_WIDTH-1:2]  wb_adr_i,
  output  reg   [31:0] 	          wb_dat_o,
  input   wire  [31:0] 	          wb_dat_i,
  input   wire 		                wb_we_i,
  input   wire  [3:0] 	          wb_sel_i,
  
  input   wire                    wb_stb_i,
  output  reg 		                wb_ack_o
);

// Calculate depth from address width
localparam DEPTH = 1 << ADDR_WIDTH;

reg [31:0] 		mem [0:DEPTH/4-1] /* verilator public */;

// Initialize the contents of memory from an hex file
initial begin
  if(|MEM_FILE) begin
    $readmemh(MEM_FILE, mem);
  end
end

wire [3:0] 		        we    = {4{wb_we_i & wb_stb_i}} & wb_sel_i;
wire [ADDR_WIDTH-3:0] addr  = wb_adr_i[ADDR_WIDTH-1:2];

// Set Ack_o
always @(posedge wb_clk_i) begin
  if (wb_rst_i)
    wb_ack_o <= 1'b0;
  else
    wb_ack_o <= wb_stb_i & !wb_ack_o;
end

// Handle Reads & Writes
always @(posedge wb_clk_i) begin
  if (we[0]) mem[addr][7:0]   <= wb_dat_i[7:0];
  if (we[1]) mem[addr][15:8]  <= wb_dat_i[15:8];
  if (we[2]) mem[addr][23:16] <= wb_dat_i[23:16];
  if (we[3]) mem[addr][31:24] <= wb_dat_i[31:24];
  
  wb_dat_o <= mem[addr];
end

endmodule