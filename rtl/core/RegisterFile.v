////////////////////////////////////////////////////////////////////   
//  RISC-V Atom 
//
//  File        : RegisterFile.v
//  Author      : Saurabh Singh (saurabh.s99100@gmail.com)
//  Description : Parametrized register file for RISCV atom core
////////////////////////////////////////////////////////////////////

`include "../Timescale.vh"
`include "Utils.vh"
`include "Defs.vh"

`default_nettype none

module RegisterFile  #(    
    parameter	REG_WIDTH = 16, 
    parameter	REG_ADDR_WIDTH =  4
) (
    ////////////////////  PORTLIST  /////////////////////

    // read port 1 (Asynchronous Read)
    input	wire    [REG_ADDR_WIDTH-1:0]	Ra_Sel_i,
    output  wire	[REG_WIDTH-1:0]         Ra_o,
    
    // read port 2 (Asynchronous Read)
    input   wire	[REG_ADDR_WIDTH-1:0]   	Rb_Sel_i,
    output  wire    [REG_WIDTH-1:0]        	Rb_o,

    // write port (Synchronous Write)
    input   wire                            Data_We_i,
    input   wire    [REG_ADDR_WIDTH-1:0]    Rd_Sel_i,
    input   wire    [REG_WIDTH-1:0]         Data_i,

    input   wire    Clk_i, 
    input   wire    Rst_i
);

    localparam REG_COUNT = 2**REG_ADDR_WIDTH;

    `ifdef RF_R0_IS_ZERO
        reg	[REG_WIDTH-1:0]	regs	[1:REG_COUNT-1] /*verilator public*/;    // register array
    `else
        reg [REG_WIDTH-1:0]	regs	[0:REG_COUNT-1] /*verilator public*/;    // register array
    `endif
    
    integer i;

    /* === WRITE PORT ===
        Synchronous write
    */
    always @ (posedge Clk_i) begin
        if (Rst_i) begin
            for(i=1; i<REG_COUNT; i=i+1)
                regs[i] <= {REG_WIDTH{1'b0}};
        end
        else if(Data_We_i) begin
            `ifdef RF_R0_IS_ZERO
                if(Rd_Sel_i != 0)
                    regs[Rd_Sel_i] <= Data_i;
            `else
                regs[Rd_Sel_i] <= Data_i;
            `endif
        end
    end

    /* === READ PORTS ===
        Asynchronous read
    */
    `ifdef RF_R0_IS_ZERO
        assign	Ra_o = (Ra_Sel_i == 0) ? {REG_WIDTH{1'b0}} : regs[Ra_Sel_i];
        assign	Rb_o = (Rb_Sel_i == 0) ? {REG_WIDTH{1'b0}} : regs[Rb_Sel_i];
    `else
        assign	Ra_o = regs[Ra_Sel_i];
        assign	Rb_o = regs[Rb_Sel_i];
    `endif

endmodule