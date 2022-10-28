////////////////////////////////////////////////////////////////////   
//  RISC-V Atom 
//
//  File        : RegisterFile.v
//  Author      : Saurabh Singh (saurabh.s99100@gmail.com)
//  Description : Parametrized register file for RISCV atom core
////////////////////////////////////////////////////////////////////
`default_nettype none

module RegisterFile  #(
    parameter	REG_WIDTH = 32,     // Width of each register
    parameter	NUM_REGS =  32,     // Number of registers
    parameter	R0_IS_ZERO = 1      // is R0 hard-wired to zero
) (
    input   wire    Clk_i, 
    input   wire    Rst_i,

    // read port 1 (Asynchronous Read)
    input	wire    [`CLOG2(NUM_REGS)-1:0]	Ra_Sel_i,
    output  wire	[REG_WIDTH-1:0]         Ra_o,
    
    // read port 2 (Asynchronous Read)
    input   wire	[`CLOG2(NUM_REGS)-1:0] 	Rb_Sel_i,
    output  wire    [REG_WIDTH-1:0]        	Rb_o,

    // write port (Synchronous Write)
    input   wire                            Data_We_i,
    input   wire    [`CLOG2(NUM_REGS)-1:0]  Rd_Sel_i,
    input   wire    [REG_WIDTH-1:0]         Data_i
);

    reg	[REG_WIDTH-1:0]	regs	[0:NUM_REGS-1] /*verilator public*/;

    integer i;

    /* === WRITE PORT ===
        Synchronous write
    */
    always @ (posedge Clk_i) begin
        if (Rst_i) begin
            for(i=0;  i<NUM_REGS; i=i+1)
                regs[i] <= {REG_WIDTH{1'b0}};
        end
        else if(Data_We_i) begin
            if(R0_IS_ZERO) begin
                if(Rd_Sel_i != 0)
                    regs[Rd_Sel_i] <= Data_i;
            end else begin
                regs[Rd_Sel_i] <= Data_i;
            end
        end
    end

    /* === READ PORTS ===
        Asynchronous read
    */
    assign	Ra_o = regs[Ra_Sel_i];
    assign	Rb_o = regs[Rb_Sel_i];
endmodule
