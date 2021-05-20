`include "AtomRV.v"
module faketb;
    reg clk = 0;
    reg rst = 0;
    reg hlt = 0;

    reg [31:0] imem [0:1000];
    initial $readmemh("imem.txt",imem);
    initial begin
        $dumpfile("out.vcd");
        $dumpvars(0, faketb);

    //    imem[0] = 32'd11;
    //    imem[1] = 32'd22;
    //    imem[2] = 32'd33;
    //    imem[3] = 32'd44;
        
        #2; rst = 1;
        #2; rst = 0;
        
        #100;
        
        $finish();
    end 

    wire [31:0] imem_address;


    // NOT USED
    wire [31:0] d_addr_o;
    wire [31:0] d_data_i;
    wire [31:0] d_data_o;
    wire d_we_o;

    AtomRV arv
    (
        .clk_i(clk),    // clock
        .rst_i(rst),    // reset
        .hlt_i(hlt),    // hault cpu

        .i_data_i(imem[imem_address-3]),   // IMEM data
        .i_addr_o(imem_address),   // IMEM Address
        .d_addr_o(d_addr_o),   // DMEM address
        .d_data_i(d_data_i),   // DMEM data in
        .d_data_o(d_data_o),   // DMEM data out
        .d_we_o(d_we_o)      // DMEM WriteEnable
    );


    always begin
        #1; clk = !clk;
    end
endmodule