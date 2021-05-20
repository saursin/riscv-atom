////////////////////////////////////////////////////////////////////   
//  RISC-V Atom 
//
//  File        : Memory.v
//  Author      : Saurabh Singh (saurabh.s99100@gmail.com)
//  Desciption  : Memory module for RISCV atom core
////////////////////////////////////////////////////////////////////
`default_nettype none

`include "../Timescale.vh"

module Memory #(parameter ADDR_WIDTH = 16, parameter LINE_WIDTH = 16)
(
    /////////////// PORT LIST //////////////////

    // Read Port    (ASYNC)
    input       [ADDR_WIDTH-1 : 0]  addr_i,
    output      [LINE_WIDTH-1 : 0]  rd_data_o,

    // Write Port   (SYNC)
    input       [LINE_WIDTH-1 : 0]  wr_data_i,
    input                           we_i,

    input                           clk_i
);

    localparam DEPTH = (1<<ADDR_WIDTH);
    
    /* MEMORY */
    reg [LINE_WIDTH-1:0]    memory [0:DEPTH-1];
    
    /*  
        Initializes all memoy locations with zeroes.
        Useful for Behavioual simulations    
    */
    initial begin
        `ifdef INITIALIZE_MEM
            integer i;
            for(i=0; i<DEPTH; i=i+1)
                memory[i] = `MEM_INITIALIZATION_VALUE;
        `endif
    
        `ifdef INITIALIZE_MEM_WITH_FILE
            // Initialize memory from a file
            $readmemh(`MEM_INIT_PATH, memory);
        `endif
    end

    always @(posedge clk_i) begin
        if (we_i) begin
            memory[addr_i] <= wr_data_i;    // Follows : Read then Write
        end
    end
    assign rd_data_o = memory[addr_i];
endmodule