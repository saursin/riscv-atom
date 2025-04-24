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
    parameter NUM_PINS = 16         // Number of Pins
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

    input   wire    [NUM_PINS-1:0] inp_i,
    output  wire    [NUM_PINS-1:0] out_o,
    output  wire    [NUM_PINS-1:0] oe_o
);

// Set Ack_o
always @(posedge wb_clk_i) begin
  if (wb_rst_i)
    wb_ack_o <= 1'b0;
  else
    wb_ack_o <= wb_stb_i & !wb_ack_o;
end

wire    [3:0]   we  = {4{wb_we_i & wb_stb_i}} & wb_sel_i;

// Holds GPIO pin state to output
reg     [NUM_PINS-1:0]  gpio_state;
assign out_o = gpio_state;

// Holds GPIO direction 
reg     [NUM_PINS-1:0]  gpio_dir;
assign oe_o = gpio_dir;

// Handle Writes
integer i;
always @(posedge wb_clk_i) begin
    if(wb_rst_i) begin
        gpio_state  <= {NUM_PINS{1'b0}};
        gpio_dir    <= {NUM_PINS{1'b0}};
    end
    else begin
        case(wb_adr_i)
            2'b00: begin   // DAT
                for(i=0; i<NUM_PINS; i=i+1) begin
                    if (we[i/8])  gpio_state[i] <= wb_dat_i[i];
                end
            end
            2'b01: begin   // TSC
                for(i=0; i<NUM_PINS; i=i+1) begin
                    if (we[i/8])  gpio_dir[i] <= wb_dat_i[i];
                end
            end
            default: begin   // INC  (NOT IMPLEMENTED)
            end
        endcase
    end
end


// READS
always @(*) /* COMBINATORIAL */ begin
    case(wb_adr_i)
        2'b00:  wb_dat_o = {{32-NUM_PINS{1'b0}}, inp_i};
        2'b01:  wb_dat_o = gpio_dir;
        default:
                wb_dat_o = gpio_dir;        // INC (Not implemented)
    endcase
end

endmodule
