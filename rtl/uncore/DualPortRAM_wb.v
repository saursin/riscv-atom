////////////////////////////////////////////////////////////////////   
//  File        : DualPortRAM.v
//  Author      : Saurabh Singh (saurabh.s99100@gmail.com)
//  Description : A Wishbone interfaced RAM with two ports, Port 1 
//    is a Read/Write port while, Port 2 is a readonly port
////////////////////////////////////////////////////////////////////
`default_nettype none

module DualPortRAM_wb #(
    // Parameters
    parameter ADDR_WIDTH = 16,
    parameter MEM_FILE = ""
)
(
    // Global Signals
    input   wire                wb_clk_i,
    input   wire 		            wb_rst_i,
    
    // Wishbone Interface (Read & Write)
    input   wire  [ADDR_WIDTH-1:2]  wb_adr_i,
    output  reg   [31:0] 	          wb_dat_o,
    input   wire  [31:0] 	          wb_dat_i,
    input   wire 		                wb_we_i,
    input   wire  [3:0] 	          wb_sel_i,
    input   wire                    wb_stb_i,
    output  reg 		                wb_ack_o,

    // Wishbone Interface (Read Only)
    input   wire  [ADDR_WIDTH-1:2]  wb_roport_adr_i,
    output  reg   [31:0] 	          wb_roport_dat_o,
    input   wire                    wb_roport_stb_i,
    output  reg		                  wb_roport_ack_o
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

//////////////////////////////////////////////////////
// Read Only (RO) Port Logic

wire [ADDR_WIDTH-3:0] roport_addr  = wb_roport_adr_i[ADDR_WIDTH-1:2];

// Set Ack_o
always @(posedge wb_clk_i) begin
  if(wb_rst_i)
    wb_roport_ack_o <= 0;
  else
    wb_roport_ack_o <= wb_roport_stb_i && !wb_roport_ack_o;
end


// Handle Reads
always @(posedge wb_clk_i) begin
  if(wb_rst_i)
    wb_roport_dat_o <= 0;
  else
    wb_roport_dat_o <= mem[roport_addr];
end

endmodule
