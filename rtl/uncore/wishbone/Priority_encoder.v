/*

Copyright (c) 2014-2021 Alex Forencich

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

// Language: Verilog 2001
`default_nettype none
// `timescale 1ns / 1ps

/*
 * Priority encoder module
 */
module Priority_encoder #
(
    parameter WIDTH = 2,
    // LSB priority selection
    parameter LSB_HIGH_PRIORITY = 0
)
(
    input  wire [WIDTH-1:0]         input_unencoded,
    output wire                     output_valid,
    output wire [$clog2(WIDTH)-1:0] output_encoded,
    output wire [WIDTH-1:0]         output_unencoded
);

parameter LEVELS = WIDTH > 2 ? $clog2(WIDTH) : 1;       // INFO: ISE complains about $clog2 in localparam
parameter W = 2**LEVELS;

// pad input to even power of two
wire [W-1:0] input_padded = {{W-WIDTH{1'b0}}, input_unencoded};

/* verilator lint_off UNUSEDSIGNAL */
wire [W/2-1:0] stage_valid[LEVELS-1:0];
/* verilator lint_on UNUSEDSIGNAL */

/* verilator lint_off UNDRIVEN */
wire [W/2-1:0] final_stage_valid[LEVELS-1:0];
/* verilator lint_on UNDRIVEN */

/* verilator lint_off UNUSEDSIGNAL */
wire [W/2-1:0] stage_enc[LEVELS-1:0];
/* verilator lint_on UNUSEDSIGNAL */

/* verilator lint_off UNDRIVEN */
wire [W/2-1:0] final_stage_enc[LEVELS-1:0];
/* verilator lint_on UNDRIVEN */


generate
    genvar l, n;

    // process input bits; generate valid bit and encoded bit for each pair
    for (n = 0; n < W/2; n = n + 1) begin : loop_in
        assign stage_valid[0][n] = |input_padded[n*2+1:n*2];
        if (LSB_HIGH_PRIORITY) begin
            // bit 0 is highest priority
            assign stage_enc[0][n] = !input_padded[n*2+0];
        end else begin
            // bit 0 is lowest priority
            assign stage_enc[0][n] = input_padded[n*2+1];
        end
    end

    if(LEVELS > 1) begin
        // compress down to single valid bit and encoded bus
        for (l = 1; l < LEVELS; l = l + 1) begin : loop_levels
            for (n = 0; n < W/(2*2**l); n = n + 1) begin : loop_compress
                assign final_stage_valid[l][n] = |stage_valid[l-1][n*2+1:n*2];
                if (LSB_HIGH_PRIORITY) begin
                    // bit 0 is highest priority
                    assign final_stage_enc[l][(n+1)*(l+1)-1:n*(l+1)] = stage_valid[l-1][n*2+0] ? {1'b0, stage_enc[l-1][(n*2+1)*l-1:(n*2+0)*l]} : {1'b1, stage_enc[l-1][(n*2+2)*l-1:(n*2+1)*l]};
                end else begin
                    // bit 0 is lowest priority
                    assign final_stage_enc[l][(n+1)*(l+1)-1:n*(l+1)] = stage_valid[l-1][n*2+1] ? {1'b1, stage_enc[l-1][(n*2+2)*l-1:(n*2+1)*l]} : {1'b0, stage_enc[l-1][(n*2+1)*l-1:(n*2+0)*l]};
                end
            end
        end
    end else begin
        assign final_stage_valid[0][0] = stage_valid[0][0];
        assign final_stage_enc[0][0] = LSB_HIGH_PRIORITY ? ~stage_enc[0][0] : stage_enc[0][0];
    end
endgenerate

assign output_valid = final_stage_valid[LEVELS-1][0];
assign output_encoded = final_stage_enc[LEVELS-1];
assign output_unencoded = 1 << output_encoded;

endmodule
