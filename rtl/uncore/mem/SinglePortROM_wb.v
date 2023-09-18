////////////////////////////////////////////////////////////////////   
//  File        : SinglePortROM.v
//  Author      : Saurabh Singh (saurabh.s99100@gmail.com)
//  Description : A Wishbone interfaced ROM module
////////////////////////////////////////////////////////////////////
`default_nettype none

module SinglePortROM_wb #(
    // Parameters
    parameter ADDR_WIDTH = 16,
    parameter DATA_WIDTH = 32,
    parameter MEM_FILE = ""
)(
    input   wire                    wb_clk_i,
    input   wire 		            wb_rst_i,
    input   wire  [ADDR_WIDTH-1:2]  wb_adr_i,
    output  reg   [DATA_WIDTH-1:0]  wb_dat_o,
    input   wire                    wb_stb_i,
    output  reg		                wb_ack_o
);

    // Calculate depth from address width
    localparam DEPTH = 1 << ADDR_WIDTH;

    reg [DATA_WIDTH-1:0] mem [0:DEPTH/4-1] /* verilator public */;

    // Initialize the contents of memory from a hex file
    initial begin
        if(|MEM_FILE) begin
            $readmemh(MEM_FILE, mem);
        end
    end

    wire [ADDR_WIDTH-3:0] addr  = wb_adr_i[ADDR_WIDTH-1:2];

    // Set Ack_o
    always @(posedge wb_clk_i) begin
        if (wb_rst_i)
            wb_ack_o <= 1'b0;
        else
            wb_ack_o <= wb_stb_i & !wb_ack_o;
    end

    // Handle Reads
    always @(posedge wb_clk_i) begin
        wb_dat_o <= mem[addr];
    end

endmodule
