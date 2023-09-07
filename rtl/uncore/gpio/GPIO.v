////////////////////////////////////////////////////////////////////   
//  File        : GPIO.v
//  Author      : Saurabh Singh (saurabh.s99100@gmail.com)
//  Description : GPIO IP with Wishbone B-4 Interface.
//
// MIT License
//
// Copyright (c) 2021 Saurabh Singh
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
////////////////////////////////////////////////////////////////////

`default_nettype none
`include "Utils.vh"

module GPIO #(
    parameter N = 16                // Number of Pins
)(
    // Wishbone Interface
    input   wire            wb_clk_i,
    input   wire            wb_rst_i,

    input   wire    [3:2]   wb_adr_i,
    output  reg     [31:0]  wb_dat_o,
    input   wire    [31:0]  wb_dat_i,
    input   wire            wb_we_i,
    input   wire    [3:0]   wb_sel_i,
    input   wire            wb_stb_i,
    output  reg             wb_ack_o,

    inout  wire     [N-1:0] gpio_io
);

// Set Ack_o
always @(posedge wb_clk_i) begin
  if (wb_rst_i)
    wb_ack_o <= 1'b0;
  else
    wb_ack_o <= wb_stb_i & !wb_ack_o;
end

wire    [3:0]   we  = {4{wb_we_i & wb_stb_i}} & wb_sel_i;

// Reflects current state of GPIO pins as inputs
wire    [N-1:0] gpio_read_val;

// Holds GPIO pin state to output
reg     [31:0]  gpio_state = 32'd0;

// Holds GPIO direction 
reg     [31:0]  gpio_dir = 32'd0;

`UNUSED_VAR(gpio_state)
`UNUSED_VAR(gpio_dir)

genvar i;
generate 
    for(i=0; i<N; i=i+1) begin: bufs
        IOBuf io
        (
            .dir_i  (gpio_dir[i]),
            .bit_i  (gpio_state[i]),
            .bit_o  (gpio_read_val[i]),
            .pin_io (gpio_io[i])
        );
    end
endgenerate


// Handle Writes
always @(posedge wb_clk_i) begin
    if(wb_rst_i) begin
        gpio_state <= 32'd0;
        gpio_dir <= 32'd0;
    end
    else begin
        case(wb_adr_i)
            2'b00: begin   // DAT
                    if (we[0])  gpio_state[7:0]      <= wb_dat_i[7:0];
                    if (we[1])  gpio_state[15:8]     <= wb_dat_i[15:8];
                    if (we[2])  gpio_state[23:16]    <= wb_dat_i[23:16];
                    if (we[3])  gpio_state[31:24]    <= wb_dat_i[31:24];
                end
            2'b01: begin   // TSC
                    if (we[0])  gpio_dir[7:0]        <= wb_dat_i[7:0];
                    if (we[1])  gpio_dir[15:8]       <= wb_dat_i[15:8];
                    if (we[2])  gpio_dir[23:16]      <= wb_dat_i[23:16];
                    if (we[3])  gpio_dir[31:24]      <= wb_dat_i[31:24];
                end
            default: begin   // INC  (NOT IMPLEMENTED)
            end
        endcase
    end
end


// READS
localparam M = 32-N;
always @(*) /* COMBINATORIAL */ begin
    case(wb_adr_i)
        2'b00:  wb_dat_o = {{M{1'b0}}, gpio_read_val};
        2'b01:  wb_dat_o = gpio_dir;
        default:
                wb_dat_o = gpio_dir;        // INC (Not implemented)
    endcase
end

endmodule
