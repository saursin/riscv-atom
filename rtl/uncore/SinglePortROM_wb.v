////////////////////////////////////////////////////////////////////   
//  File        : SinglePortROM.v
//  Author      : Saurabh Singh (saurabh.s99100@gmail.com)
//  Description : A Wishbone interfaced ROM module
////////////////////////////////////////////////////////////////////
`default_nettype none

module SinglePortROM_wb #(
  // Parameters
  parameter ADDR_WIDTH = 16,
  parameter MEM_INIT_FILE = ""
)
(
  // Generic Handshaking Interface
  input   wire                    clk_i,
  input   wire 		                rst_i,

  input   wire  [ADDR_WIDTH-1:2]  addr_i,
  output  reg   [31:0] 	          data_o,

  // Handshake Signals
  input   wire                    stb_i,
  output  reg		                  ack_o
);

// Calculate depth from address width
localparam DEPTH = 1 << ADDR_WIDTH;

reg [31:0] 		mem [0:DEPTH/4-1] /* verilator public */;

// Initialize the contents of memory from a hex file
initial begin
  if(|MEM_INIT_FILE) begin
    $readmemh(MEM_INIT_FILE, mem);
  end
end

wire [ADDR_WIDTH-3:0] addr  = addr_i[ADDR_WIDTH-1:2];

// Set Ack_o
always @(posedge clk_i) begin
  if(rst_i)
    ack_o <= 0;
  else
    ack_o <= stb_i && !ack_o;
end


// Handle Reads
always @(posedge clk_i) begin
  if(rst_i)
    data_o <= 0;
  else
    data_o <= mem[addr];
end

endmodule